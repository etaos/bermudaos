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

//! \file src/dev/i2c/busses/atmega-i2c.c ATmega I2C bus driver.

#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/i2c.h>
#include <dev/i2c-reg.h>
#include <dev/i2c-core.h>
#include <dev/i2c-msg.h>
#include <dev/i2c/busses/atmega.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#include <fs/vfile.h>

#include <arch/io.h>
#include <arch/irq.h>

#include "atmega_priv.h"

/* static functions */
static void atmega_i2c_ioctl(struct device *dev, int cfg, void *data);
static unsigned char atmega_i2c_calc_twbr(uint32_t freq, unsigned char pres);
static unsigned char atmega_i2c_calc_prescaler(uint32_t frq);

static struct i2c_message *i2c_init_transfer(struct i2c_adapter *adap, uint32_t freq);
static int i2c_resume_transfer(struct i2c_adapter *adapter);
static int i2c_master_transfer(struct i2c_adapter *adapter);
static int i2c_slave_transfer(struct i2c_adapter *adapter);

/**
 * \brief File I/O structure for bus 0 on port C.
 */
static FDEV_SETUP_STREAM(i2c_c0_io, NULL, NULL, NULL, 
						 NULL, NULL, 
						 I2C_FNAME, 0 /* flags */, NULL /* data */);

/**
 * \brief I2C control structure for the bus 0 on port C.
 */
static struct atmega_i2c_priv i2c_c0 = {
	.twcr = &TWCR,
	.twdr = &TWDR,
	.twsr = &TWSR,
	.twar = &TWAR,
	.twbr = &TWBR,
	.twamr = &TWAMR,
};

#ifdef __THREADS__
/**
 * \brief I2C bus 0 mutex.
 */
static volatile void *bus_c0_mutex = SIGNALED;
/**
 * \brief I2C bus 0 master queue.
 */
static volatile void *bus_c0_master_queue = SIGNALED;
/**
 * \brief I2C bus 0 slave queue.
 */
static volatile void *bus_c0_slave_queue = SIGNALED;
#endif

/**
 * \brief Array of all available I2C busses
 */
struct i2c_adapter *atmega_i2c_busses[ATMEGA_BUSSES];

/**
 * \brief Index of the current message.
 * 
 * current_index holds the index of the message that is currently being processed.
 */
static volatile size_t current_index = 0;

/**
 * \brief Intialize an I2C bus.
 * \param sla Slave address of this bus.
 * \param adapter Bus adapter.
 * 
 * This function will initialize bus 0 on port c.
 */
PUBLIC void atmega_i2c_c0_hw_init(uint8_t sla, struct i2c_adapter *adapter)
{
	int rc = i2c_init_adapter(adapter, i2c_c0_io.name);
	if(rc < 0) {
		return;
	}
	
	atmega_i2c_busses[0] = adapter;
#ifdef __THREADS__
	adapter->dev->mutex = &bus_c0_mutex;
#endif
	adapter->dev->ctrl = &atmega_i2c_ioctl;
	adapter->dev->io = &i2c_c0_io;

#ifdef __THREADS__
	adapter->master_queue = &bus_c0_master_queue;
	adapter->slave_queue = &bus_c0_slave_queue;
#else
	adapter->mutex = 0;
	adapter->master_queue = 1;
	adapter->slave_queue = 1;
#endif
	adapter->data = (void*)&i2c_c0;
	adapter->features = I2C_MASTER_SUPPORT | I2C_SLAVE_SUPPORT;
	adapter->start_transfer = &i2c_init_transfer;
	adapter->resume = &i2c_resume_transfer;

	vfs_add(&i2c_c0_io);
	open(i2c_c0_io.name, _FDEV_SETUP_RW);
	
	adapter->dev->ctrl(adapter->dev, I2C_IDLE, NULL);
	atmega_i2c_reg_write(i2c_c0.twar, &sla);
}

/**
 * \brief Initialise a I2C client structure.
 * \param client Client to initialize.
 * \param ifac BUs to initialize the client for.
 */
PUBLIC void atmega_i2c_init_client(struct i2c_client *client, uint8_t ifac)
{
	client->adapter = atmega_i2c_busses[ifac];
}

/**
 * \brief Calculate the value of the TWBR register.
 * \param freq Wanted frequency.
 * \param pres Used prescaler.
 * \note The <b>pres</b> parameter can have one of the following values: \n
 *       * I2C_PRES_1 \n
 *       * I2C_PRES_4 \n
 *       * I2C_PRES_16 \n
 *       * I2C_PRES_64
 * \see I2C_PRES_1
 * \see I2C_PRES_4 
 * \see I2C_PRES_16 
 * \see I2C_PRES_64
 * 
 * The needed value of the TWBR register will be calculated using the given
 * (and used) prescaler value.
 */
static unsigned char atmega_i2c_calc_twbr(uint32_t freq, unsigned char pres)
{
	char prescaler;
	uint32_t twbr;
	
	switch(pres) {
		case I2C_PRES_1:
			prescaler = 1;
			break;
		case I2C_PRES_4:
			prescaler = 4;
			break;
		case I2C_PRES_16:
			prescaler = 16;
			break;
		case I2C_PRES_64:
			prescaler = 64;
			break;
		default:
			prescaler = -1;
			break;
	}
	
	if(prescaler == -1) {
		return 0xFF;
	}
	
	twbr = I2C_CALC_TWBR(freq, prescaler);
	
	return twbr & 0xFF;
}

/**
 * \brief Calculates the I2C prescaler value.
 * \param frq The desired frequency.
 * \return The prescaler value in hardware format: \n
 *         * B0 for a prescaler of 1; \n
 *         * B1 for a prescaler of 4; \n
 *         * B10 for a prescaler of 16; \n
 *         * B11 for a prescaler of 64
 *
 * Calculates the prescaler value based on the given frequency.
 */
static unsigned char atmega_i2c_calc_prescaler(uint32_t frq)
{
	unsigned char ret = 0;
	
	if(frq > I2C_FRQ(255,1)) {
		ret = B0;
	}
	else if(frq > I2C_FRQ(255,4) && frq < I2C_FRQ(1,4)) {
		ret = B11;
	}
	else if(frq > I2C_FRQ(255,16) && frq < I2C_FRQ(1,16)) {
		ret = B10;
	}
	else if(frq > I2C_FRQ(255,64) && frq < I2C_FRQ(1,64)) {
		ret = B1;
	}

	return ret;
}

/**
 * \brief ATmega I2C bus controller.
 * \param dev Bus device.
 * \param cfg Config options.
 * \param data Configuration data.
 */
static void atmega_i2c_ioctl(struct device *dev, int cfg, void *data)
{
	struct i2c_adapter *adap = dev->ioctl;
	struct atmega_i2c_priv *priv = adap->data;
	uint8_t reg = 0;
	
	atmega_i2c_reg_read(priv->twcr, &reg);
	
	switch((uint16_t)cfg) {
		/* transaction control */
		case I2C_START | I2C_ACK:
			reg |= BIT(TWSTA) | BIT(TWINT) | BIT(TWEA) | BIT(TWEN) | BIT(TWIE);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_START | I2C_NACK:
			reg = BIT(TWSTA) | BIT(TWINT) | BIT(TWIE) | BIT(TWEN) | (reg & ~BIT(TWEA));
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_STOP | I2C_ACK:
			reg |= BIT(TWSTO) | BIT(TWINT) | BIT(TWEA) | BIT(TWEN) | BIT(TWIE);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_STOP | I2C_NACK:
			reg = (reg & ~BIT(TWEA)) | BIT(TWSTO) | BIT(TWINT) | BIT(TWEN) | BIT(TWIE);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
		
		/* bus control */
		case I2C_RELEASE:
			reg |= BIT(TWEN) | BIT(TWIE) | BIT(TWINT);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_ACK:
			reg |= BIT(TWINT) | BIT(TWEA) | BIT(TWEN) | BIT(TWIE);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_NACK:
			reg = (reg & ~BIT(TWEA)) | BIT(TWINT) | BIT(TWEN) | BIT(TWIE);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;

		case I2C_BLOCK:
			reg &= ~(BIT(TWINT) | BIT(TWIE));
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		case I2C_RESET:
			reg |= BIT(TWSTO);
			atmega_i2c_reg_write(priv->twcr, &reg);
			break;
			
		default:
			break;
	}
	return;
}

static struct i2c_message *i2c_init_transfer(struct i2c_adapter *adapter, uint32_t freq)
{
	struct i2c_message *first;
	current_index = 0;
	int rc;
	
	first = i2c_vector_get(adapter, current_index);
	if(first) {
		if(neg(i2c_msg_features(first)) & I2C_MSG_MASTER_MSG_MASK) {
			uint8_t pres = atmega_i2c_calc_prescaler(freq);
			uint8_t twbr = atmega_i2c_calc_twbr(freq, pres);
			TWBR = twbr;
			TWSR = pres & B11;
			rc = i2c_master_transfer(adapter);
		} else {
			/* slave msg */
			rc = i2c_slave_transfer(adapter);
		}
		
		if(rc >= 0) {
			/* no error */
			return i2c_vector_get(adapter, current_index);
		}
	}
	return NULL;
}

static int i2c_master_transfer(struct i2c_adapter *adapter)
{
	return -1;
}

static int i2c_slave_transfer(struct i2c_adapter *adapter)
{
	return -1;
}

static int i2c_resume_transfer(struct i2c_adapter *adapter)
{
	return -1;
}
