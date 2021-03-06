/*
 *  BermudaOS - SPI device interface
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

#ifndef __SPI_DEV_h
#define __SPI_DEV_h

#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/error.h>

/**
 * \brief Field in spi_adapter::features.
 */
#define SPI_MASTER_SUPPORT B1

/**
 * \brief Field in spi_adapter::features.
 */
#define SPI_SLAVE_SUPPORT B10

/**
 * \brief Type definition of the SPI features (flags).
 */
typedef uint8_t spi_features_t;

/**
 * \brief SPI master feature.
 */
#define SPI_MASTER 0x100
/**
 * \brief SPI slave feature.
 */
#define SPI_SLAVE  0x200

/**
 * \brief 1MHz SPI transfer.
 */
#define SPI_1MHZ 10000000

/**
 * \brief Shared info structure
 */
struct spi_shared_info
{
	reg8_t cs; //!< Chip select register.
	uint8_t cspin; //! Chip select pin.
	uint32_t freq; //!< Operation frequency.
} __attribute__((packed));

/**
 * \brief SPI bus adapter.
 */
struct spi_adapter
{
	struct device *dev; //!< I/O device.
	bool busy; //!< Busy flag.
	uint8_t error; //!< Bus error field.
	spi_features_t features; //!< Bus features.
	int (*xfer)(struct spi_adapter*, struct spi_shared_info*);
	
	volatile uint8_t *buff; //!< I/O buffer.
	size_t length; //!< Length of TX and RX.
} __attribute__((packed));

/**
 * \brief Represents the SPI based chip.
 */
struct spi_client
{
	struct spi_adapter *adapter; //!< Bus adapter.
	
	reg8_t cs; //!< Chip select register.
	uint8_t cspin; //! Chip select pin.
	uint32_t freq; //!< Operation frequency.
	
	FILE *stream; //!< I/O file.
	uint8_t *buff; //!< I/O buffer.
	size_t length; //!< Length of tx and rx.
} __attribute__((packed));

__DECL
extern int spidev_socket(struct spi_client *client, uint16_t flags);
extern int spidev_write(FILE *stream, const void *tx, size_t size);
extern int spidev_read(FILE *stream, void *rx, size_t size);
extern int spidev_close(FILE *stream);
extern int spidev_flush(FILE *stream);

/* found in spi-core.c */
struct spi_client *spi_alloc_client(struct spi_adapter *adapter, reg8_t reg, uint8_t cs,
										   uint32_t freq);
__DECL_END

#endif
