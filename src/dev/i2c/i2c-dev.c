/*
 *  BermudaOS - I2C device driver
 *  Copyright (C) 2012   Michel Megens <dev@michelmegens.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file src/dev/i2c/i2c-dev.c I2C device driver
 *
 * \addtogroup i2c I2C module
 * \brief Universal I2C driver.
 * 
 * \section Unix Unix
 * This driver uses a file based unix interface. This means that all communication is done using
 * file I/O. The only differency is the opening of files. Normally the (f)open function is used for
 * this purpose, but in this case i2cdev_socket is used.
 * 
 * \section io I/O and error handling
 * <b>Master read/write</b>
 * When you want to do a master transfer, sent a repeated start and receive data from the slave
 * your I/O block will look like the following.
 * 
\code{.c}
struct i2c_client client;
atmega_i2c_init_client(&client, ATMEGA_I2C_C0);
client.sla = 0x54;      // slave address
client.freq = 100000UL; // frequency in hertz
int rc, fd;

fd = i2cdev_socket(&client, _FDEV_SETUP_RW | I2C_MASTER);
if(fd < 0) {
    error();
}

rc = write(fd, txbuff, txbuff_length);
rc += read(fd, rxbuff, rxbuff_length);

if(rc == 0) {
    rc = flush(fd);
} else {
    i2cdev_error(fd);
}
  
close(fd);
\endcode
 * 
 * If you want to do a transmit or receive only you should set the correct buffer to <i>NULL</i>. The
 * transmit buffer will be set using <i>write</i> the receive buffer is set using <i>read</i>.
 * 
 * <b>Slave recieve/transmit</b>
\code{.c}
struct i2c_client slave_client;

atmega_i2c_init_client(&slave_client, ATMEGA_I2C_C0);
slave_client.callback = &slave_responder;
 
fd = i2cdev_socket(&slave_client, _FDEV_SETUP_RW | I2C_SLAVE);
if(fd < 0) {
    goto _usart;
}
i2cdev_listen(fd, &rx, 1);
close(fd);
\endcode
 * 
 * The slave will first wait for a master receive request. When it is finished, it will check for a
 * user callback function. That function can set the transmit buffer. Set the buffer to <i>NULL</i>
 * to not transmit any data as a slave.
 * @{
 */

#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/i2c.h>
#include <dev/i2c-reg.h>
#include <dev/i2c-core.h>

#include <sys/thread.h>
#include <sys/epl.h>
#include <sys/events/event.h>

/**
 * \brief Request an I2C I/O file.
 * \param client I2C driver client.
 * \param flags File flags.
 * \return The file descriptor associated with this client.
 */
PUBLIC int i2cdev_socket(struct i2c_client *client, uint16_t flags)
{
	FILE *socket;
	i2c_features_t features;
	struct i2c_shared_info *shinfo = i2c_shinfo(client);
	int rc = -1;
	
	if(client == NULL) {
		return -1;
	}
	
	if(BermudaEventWait(event(&(shinfo->mutex)), I2C_TMO) != 0) {
		goto out;
	}
	
	socket = BermudaHeapAlloc(sizeof(*socket));
	if(!socket) {
		rc = -1;
		goto out;
	}
	
	rc = iob_add(socket);
	if(rc < 0) {
		BermudaHeapFree(socket);
		BermudaEventSignal(event(&(shinfo->mutex)));
		goto out;
	}
	
	features = i2c_client_features(client);
	features |= I2C_CLIENT_HAS_LOCK_FLAG;
	i2c_client_set_features(client, features);
	shinfo->socket = socket;
	
	socket->data = client;
	socket->name = "I2C";
	socket->write = &i2cdev_write;
	socket->read = &i2cdev_read;
	socket->flush = &i2cdev_flush;
	socket->close = &i2cdev_close;
	socket->flags = flags;
	
	out:
	return rc;
}

/**
 * \brief Initializes the buffer for an I2C master transmit.
 * \param file I/O file.
 * \param buff The I2C message.
 * \param size Length of <i>buff</i>.
 * \return This function returns 0 on success (i.e. buffers have been allocated successfully) and -1
 *         otherwise.
 */
PUBLIC int i2cdev_write(FILE *file, const void *buff, size_t size)
{
	struct i2c_client *client = file->data;
	i2c_features_t features;
	char *layout = i2c_transmission_layout(client);
	
	if(client) {
		features = I2C_MSG_MASTER_MSG_FLAG | I2C_MSG_TRANSMIT_MSG_FLAG | I2C_MSG_SENT_REP_START_FLAG;
		if(*(++layout) == '\0') {
			features = (features & ~I2C_MSG_SENT_REP_START_FLAG) | I2C_MSG_SENT_STOP_FLAG;
		}
		
		i2c_set_transmission_layout(client, layout);
	} else {
		return -1;
	}
	
	if(i2c_write_client(client, buff, size, features) == 0) {
		return size;
	} else {
		return -1;
	}
}

/**
 * \brief Set the master receive buffer.
 * \param file The I/O file.
 * \param buff Read buffer.
 * \param size Size of \p buff.
 */
PUBLIC int i2cdev_read(FILE *file, void *buff, size_t size)
{
	struct i2c_client *client = file->data;
	i2c_features_t features;
	char *layout = i2c_transmission_layout(client);
	
	if(client) {
		features = I2C_MSG_MASTER_MSG_FLAG | I2C_MSG_SENT_REP_START_FLAG;
		if(*(++layout) == '\0') {
			features = (features & ~I2C_MSG_SENT_REP_START_FLAG) | I2C_MSG_SENT_STOP_FLAG;
		}
		i2c_set_transmission_layout(client, layout);
	} else {
		return -1;
	}

	if(i2c_write_client(client, buff, size, features) == 0) {
		return size;
	} else {
		return -1;
	}
}

/**
 * \brief Flush the I/O file.
 * \param stream File to flush.
 * \note A transfer will only be intiated if the client has messages assigned.
 * 
 * This will start the actual transfer.
 */
PUBLIC int i2cdev_flush(FILE *stream)
{
	struct i2c_client *client = stream->data;
	struct i2c_shared_info *info = i2c_shinfo(client);
	
	if(epl_entries(info->list)) {
		return i2c_flush_client(client);
	} else {
		return -1;
	}
}

/**
 * \brief Close the I2C socket.
 * \param stream File to close.
 */
PUBLIC int i2cdev_close(FILE *stream)
{
	struct i2c_client *client = stream->data;
	struct i2c_shared_info *info = i2c_shinfo(client);
	i2c_features_t features;
	
	if(epl_entries(info->list)) {
		i2c_cleanup_client_msgs(client);
	}
	
	free(stream);
	features = i2c_client_features(client);
	features &= ~I2C_CLIENT_HAS_LOCK_FLAG;
	i2c_client_set_features(client, features);
	info->socket = NULL;
	
	BermudaEventSignal(event(&info->mutex));
	return 0;
}

/**
 * \brief Setup the I2C slave interface and wait for incoming master requests.
 * \param fd File descriptor of the client.
 * \param buff Slave read buffer.
 * \param size Length of buffer.
 * 
 * Waits for a slave transfer. \p Buff will be setup as a slave receive buffer. If transmission
 * needs to be done afterwards, client::callback has to be set.
 */
PUBLIC int i2cdev_listen(int fd, void *buff, size_t size)
{
	FILE *stream = fdopen(fd);
	struct i2c_client *client;
	struct i2c_shared_info *info;
	i2c_features_t features;
	int rc;
	
	if(stream == NULL) {
		rc = -1;
		goto out;
	}
	
	client = stream->data;
	info = i2c_shinfo(client);
	
	features = I2C_MSG_SLAVE_MSG_FLAG;
	if(info->shared_callback != NULL) {
		features |= I2C_MSG_CALL_BACK_FLAG;
	}
	i2c_write_client(client, buff, size, features);
	rc = i2cdev_flush(stream);
	
	if(!rc) {
		if(info->shared_callback) {
			/* TODO: call application */
		}
	}
	
	i2c_do_clean_msgs(client->adapter);
	out:
	return rc;
}

/**
 * \brief Handle an I2C file I/O error.
 * \param fd File descriptor of the current transfer.
 * 
 * It will free the correspondending buffers. If I2C_MASTER is given all master buffers will be
 * free'd if I2C_SLAVE is passed, all slave buffers will be free'd.
 */
PUBLIC void i2cdev_error(int fd)
{
	FILE *stream = fdopen(fd);
	struct i2c_client *client = stream->data;
	
	if(epl_entries(info->list)) {
		i2c_cleanup_client_msgs(client);
	}
}

