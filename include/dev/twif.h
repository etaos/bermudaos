/*
 *  BermudaOS - TWI interface
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

//! \file dev/twif.h TWI interface

#ifndef __TWIF_H
#define __TWIF_H

#include <bermuda.h>

#include <dev/dev.h>
#include <lib/binary.h>

/**
 * \def BERMUDA_TWI_RW_SHIFT
 * \brief Bit location of the R/W bit.
 */
#define BERMUDA_TWI_RW_SHIFT 0

#define TWI_WRITE 0 //!< TWI SLA+W bit mask.
#define TWI_READ BIT(0) //!< TWI SLA+R bit mask.

/**
 * \typedef TWIMODE
 * \brief Type definition of the TWI mode.
 * \see TWIBUS
 */
typedef enum
{
	TWI_MASTER_TRANSMITTER, //!< Master transmit mode.
	TWI_MASTER_RECEIVER, //!< Master receive mode.
	TWI_SLAVE_TRANSMITTER, //!< Slave transmit mode.
	TWI_SLAVE_RECEIVER, //!< Slave receive mode.
} TWIMODE;

/**
 * \brief Type of the bus.
 * 
 * A bus can either be controlled by internal hardware, or by a software driver.
 */
typedef enum
{
	TWI_SOFTWARE_CONTROLLER, //!< Type definition of software based TWI buses.
	TWI_HARDWARE_CONTROLLER, //!< Type definition of hardware based TWI buses.
} TWI_BUS_TYPE;

/**
 * \typedef TW_IOCTL_MODE
 * \brief Type definition of I/O control modes for TWI.
 * \see BermudaTwIoctl
 */
typedef enum
{
	TW_SET_RATE, //!< Set the TWBR register.
	TW_SET_PRES, //!< Set the SCL prescaler.
	
	/**
	 * \brief Set the slave address.
	 * \note GCR will be disabled by default.
	 * \see TW_SET_GCR
	 */
	TW_SET_SLA,
	TW_SET_GCR, //!< Enable GCR.
	TW_CLEAR_GCR, //!< Disable the GCR bit.

	TW_RELEASE_BUS, //!< Release the bus.
	TW_GET_STATUS, //!< Get the status from hardware.
	TW_ENABLE_INTERFACE, //!< Enable the interface by entering idle mode.
	TW_DISABLE_INTERFACE, //!< Disable the TWI interface.
	TW_BLOCK_INTERFACE, //!< Blocks the TWI interface.
	TW_SLAVE_LISTEN, //!< Listen for incoming slave requests.

	TW_SENT_START, //!< Sent the TWI start condition.
	TW_SENT_SLA, //!< Sent the slave address over the TWI bus.
	TW_SENT_DATA, //!< Sent a data byte over the TWI bus.
	TW_SENT_STOP, //!< Sent a stop condition.
	TW_READ_DATA, //!< Read data from the bus.

	TW_REPLY_ACK, //!< Used by slave to ACK to the calling master.
	TW_REPLY_NACK, //!< Used by slave to disable ACKing.

} TW_IOCTL_MODE;

/**
 * \typedef TWIBUS
 * \brief Type definition of the TWI bus.
 */
typedef struct _twibus TWIBUS;

/**
 * \typedef TWIF
 * \brief Type definition of the TWI interface.
 */
typedef struct _twif TWIF;

struct softio
{
	// generic io registers used in twi
	reg8_t io_in; //!< SCL/SDA input register.
	reg8_t io_out; //!< SLA/SDA output register.
	
	// pins
	unsigned char scl; //!< SCL pin.
	unsigned char sda; //!< SDA pin.
} __attribute__((packed));

/**
 * \brief TWI message structure.
 * 
 * Used to pass data to the TWI front-end.
 */
typedef struct twimsg
{
	/**
	 * \brief Call back used in slave operation.
	 * \param msg Message used in the operation.
	 * 
	 * This function pointer will be called when a slave receive is done. The
	 * application can then check the received values and apply the correct
	 * transmission values.
	 */
	void (*call_back)(struct twimsg *msg);
	const unsigned char *tx_buff; //!< Transmit buffer.
	size_t tx_length; //!< Transmit buffer length.
	
	unsigned char *rx_buff; //!< Receive buffer.
	size_t rx_length; //!< Receive buffer length.
	
	uint32_t scl_freq; //!< TWI operation frequency in master mode.
	unsigned int tmo; //!< Maximum transfer waiting time-out. Used in master and slave.
	unsigned char sla; //!< Slave address to address in master mode.
} TWIMSG;

/**
 * \brief Type definition of TWI call back functions.
 */
typedef void (*twi_call_back_t)(TWIMSG *msg);

/**
 * \struct _twif
 * \brief TWI communication interface.
 * \see _twibus
 * 
 * Structure defining the communication function for a TWIBUS.
 */
struct _twif
{
#ifdef __EVENTS__
	/**
	 * \brief Function pointer to the transfer function.
	 * \param bus Bus interface to use with the transfer.
	 * \param tx Transmit buffer.
	 * \param rx Receiving buffer.
	 * \param tmo Transfer waiting time-out.
	 * 
	 * Transfer data, depending on the set TWIMODE, over the TWI interface.
	 */
	int (*transfer)(TWIBUS *twi, const void *tx, unsigned int txlen,
		void *rx, unsigned int rxlen, unsigned char sla,
		uint32_t frq, unsigned int tmo);
	int (*listen)(TWIBUS *bus, size_t *num, void *rx, size_t rxlen, 
				  unsigned int tmo);
	int (*respond)(TWIBUS *bus, const void *tx, size_t txlen,
				   unsigned int tmo);
#else
	int (*transfer)(TWIBUS *twi, const void *tx, unsigned int txlen,
		void *rx, unsigned int rxlen, unsigned char sla,
		uint32_t frq);
	int (*listen)(TWIBUS *bus, size_t *num, void *rx, size_t rxlen);
	int (*respond)(TWIBUS *bus, const void *tx, size_t txlen);
#endif
	/**
	 * \brief I/O control functions.
	 * \param bus TWI bus structure.
	 * \param mode I/O mode to set/get.
	 * \param conf I/O configuration.
	 * 
	 * Controls the I/O logic. Implemented by the TWI driver.
	 */
	int (*io)(TWIBUS *bus, TW_IOCTL_MODE mode, void *conf);
	
	/**
	 * \brief Checks the status of SCL and SDA.
	 * \param bus Bus interface to check.
	 * \return -1 if the given interface is in idle; \n
	 *         0 if SDA is low and SCL is HIGH; \n
	 *         1 if SCL is low and SDA is high; \n
	 *         2 if SCL and SDA are both low.
	 * \return The default return value is 2.
	 * 
	 * It is safe to use the interface if this function returns -1 (both lines are
	 * HIGH).
	 */
	int (*ifbusy)(TWIBUS *bus);

	/**
	 * \brief ISR handler.
	 * \param bus TWI bus.
	 * 
	 * The ISR handler which will handle the logic.
	 */
	void (*isr)(TWIBUS * bus);
} __attribute__((packed));

/**
 * \struct _twibus
 * \brief TWI bus structure.
 * \see _twif
 *
 * Each different TWI bus has its own _twibus structure.
 */
struct _twibus
{
#ifdef __EVENTS__
	volatile void *mutex; //!< TWI bus mutex.
	volatile void *master_queue; //!< TWI master waiting queue.
	volatile void *slave_queue;  //!< TWI slave waiting queue.
#else
	mutex_t mutex;
	mutex_t master_queue;
	mutex_t slave_queue;
#endif

	struct _twif *twif; //!< TWI hardware communication interface.
	union {
		void *hwio; //!< TWI hardware I/O registers.
		struct softio *softio; //!< Software TWI registers;
	} io;

	const unsigned char *master_tx; //!< TWI transmit buffer.
	size_t master_tx_len; //!< Length of the tx buffer.
	unsigned char *master_rx; //!< TWI receive buffer.
	size_t master_rx_len; //!< Length of the rx buffer.
	size_t master_index; //!< Data buffer index.

	const unsigned char *slave_tx; //!< TWI transmit buffer.
	size_t slave_tx_len; //!< Length of the tx buffer.
	unsigned char *slave_rx; //!< TWI receive buffer.
	size_t slave_rx_len; //!< Length of the rx buffer.
	size_t slave_index; //!< Data buffer index.

	TWIMODE mode; //!< TWI communication mode.
	uint8_t sla; //!< Configured slave address + R/W bit.
	uint32_t freq; //!< TWI transfer frequency in Hertz.

	unsigned char error; //!< TWI error member.
	unsigned char status; //!< TWI status
	bool busy; //!< When set to !0 the interface is busy.
} __attribute__((packed));

#endif
