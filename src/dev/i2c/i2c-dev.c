/*
 *  BermudaOS - I2C device driver
 *  Copyright (C) 2012   Michel Megens
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
 * \file src/dev/i2c/i2c-dev.c I2C device driver.
 */

#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/i2c/i2c.h>
#include <dev/i2c/reg.h>

#include <sys/thread.h>
#include <sys/events/event.h>

/**
 * \brief Initializes the given adapter.
 * \param adapter Adapter to initialize.
 * \param fname Name of the device.
 * \note Called by the bus driver.
 */
PUBLIC int i2c_init_adapter(struct i2c_adapter *adapter, char *fname)
{
	int rc = -1;
	adapter->dev = BermudaHeapAlloc(sizeof(*(adapter->dev)));
	
	if(adapter->dev == NULL) {
		return rc;
	} else {
		rc = 0;
	}
	
	adapter->dev->name = fname;
	BermudaDeviceRegister(adapter->dev, adapter);
	
	adapter->flags = 0;
	adapter->busy = false;
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
	struct i2c_message msg;
	int rc = -1;
	
	if(client != NULL) {
		msg.buff = (void*)buff;
		msg.length = size;
		msg.freq = client->freq;
		msg.addr = client->sla;
		rc = i2c_setup_msg(client->adapter->dev->io, &msg, I2C_MASTER_TRANSMIT_MSG);
	}

	return rc;
}

/**
 * \brief Set the master receive buffer.
 * \param file The I/O file.
 * \param buff Read buffer.
 * \param size Size of <i>buff</i>.
 */
PUBLIC int i2cdev_read(FILE *file, void *buff, size_t size)
{
	struct i2c_client *client = file->data;
	struct i2c_message msg;
	int rc = -1;
	
	if(client != NULL) {
		if(!buff) {
			return i2c_setup_msg(client->adapter->dev->io, NULL, I2C_MASTER_RECEIVE_MSG);
		}
		msg.buff = (void*)buff;
		msg.length = size;
		msg.freq = client->freq;
		msg.addr = client->sla;
		rc = i2c_setup_msg(client->adapter->dev->io, &msg, I2C_MASTER_RECEIVE_MSG);
	}

	return rc;
}

/**
 * \brief Request an I2C I/O file.
 * \param client I2C driver client.
 * \param flags File flags.
 */
PUBLIC int i2cdev_socket(struct i2c_client *client, uint16_t flags)
{
	struct i2c_adapter *adap;
	struct device *dev;
	FILE *socket;
	int rc = -1;
	
	if(client == NULL) {
		return -1;
	}
	
	adap = client->adapter;
	dev = adap->dev;

#ifdef __THREADS__
	if((flags & I2C_MASTER) != 0) {
		if((rc = dev->alloc(dev, I2C_TMO)) == -1) {
			rc = -1;
			goto out;
		}
	}
#else
	if((flags & I2C_MASTER) != 0) {
		BermudaMutexEnter(&(adap->mutex));
	}
#endif
	
	socket = BermudaHeapAlloc(sizeof(*socket));
	if(!socket) {
		rc = -1;
		goto out;
	}
	
	rc = iob_add(socket);
	if(rc < 0) {
		BermudaHeapFree(socket);
		dev->release(dev);
		goto out;
	}
	
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
 * \brief Flush the I/O file.
 * \param stream File to flush.
 * 
 * This will start the actual transfer.
 */
PUBLIC int i2cdev_flush(FILE *stream)
{
	struct i2c_client *client = stream->data;
	struct i2c_adapter *adap = client->adapter;
	int rc = adap->dev->io->fd;
	
	adap->dev->io->data = stream->data;
	adap->dev->io->flags &= 0xFF;
	adap->dev->io->flags |= stream->flags & 0xFF00;
	
	rc = flush(rc);
	i2c_do_clean_msgs();
	return rc;
}

/**
 * \brief Setup the I2C slave interface and wait for incoming master requests.
 */
PUBLIC int i2cdev_listen(int fd, void *buff, size_t size)
{
	FILE *stream = fdopen(fd);
	struct i2c_message msg;
	struct i2c_client *client;
	int rc, dev;
	
	if(stream == NULL) {
		rc = -1;
		goto out;
	}
	
	client = stream->data;
	dev = client->adapter->dev->io->fd;

	msg.buff = buff;
	msg.length = size;
	msg.addr = client->sla;
	msg.freq = client->freq;
	if((rc = i2c_setup_msg(fdopen(dev), &msg, I2C_SLAVE_RECEIVE_MSG)) == -1) {
		goto out;
	}
	
	fdopen(dev)->data = stream->data;
	fdopen(dev)->flags &= 0xFF;
	fdopen(dev)->flags |= stream->flags & 0xFF00;
	rc = flush(dev);
	
	if(!rc) {
		if(client->callback) {
			fdopen(dev)->data = client;
			rc = i2c_call_client(client, fdopen(dev));
		} else {
			i2c_setup_msg(fdopen(dev), NULL, I2C_SLAVE_TRANSMIT_MSG);
			rc = client->adapter->slave_respond(fdopen(dev));
		}
	}
	
	i2c_do_clean_msgs();
	out:
	return rc;
}

/**
 * \brief Close the I2C socket.
 * \param stream File to close.
 */
PUBLIC int i2cdev_close(FILE *stream)
{
	struct i2c_client *client = stream->data;
	struct i2c_adapter *adap = client->adapter;
	int rc = -1;
	
	if(stream != NULL) {
		if(stream->buff != NULL) {
			BermudaHeapFree((void*)stream->buff);
		}
		BermudaHeapFree(stream);
		rc = 0;
	}
	
#ifdef __THREADS__
	if(stream->flags & I2C_MASTER) {
		adap->dev->release(adap->dev);
	}
#else
	if(stream->flags & I2C_MASTER) {
		BermudaMutexRelease(&(adap->mutex));
	}
#endif
	return rc;
}
