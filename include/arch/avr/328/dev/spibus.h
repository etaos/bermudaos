/*
 *  BermudaOS - Serial Peripheral Interface Bus
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

/** \file arch/avr/328/dev/spibus.h */

#ifndef __SPIBUS328_H_
#define __SPIBUS328_H_

#include <bermuda.h>
#include <dev/spibus.h>
#include <arch/avr/io.h>

/**
 * \def SPI_DDR
 * \brief DDR for hardware SPI.
 */
#define SPI_DDR (*(AvrIO->ddrb))

/**
 * \def SPI_PORT
 * \brief SPI I/O port.
 */
#define SPI_PORT (*(AvrIO->portb))

/**
 * \def SPI_SCK
 * \brief SCK pin.
 */
#define SPI_SCK  5

/**
 * \def SPI_MISO
 * \brief MISO pin.
 */
#define SPI_MISO 4

/**
 * \def SPI_MOSI
 * \brief MOSI pin.
 */
#define SPI_MOSI 3

/**
 * \def SPI_SS
 * \brief SS pin.
 */
#define SPI_SS   2

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \struct _hwspi
 * \brief Hardware control structure.
 * 
 * Hardware I/O registers which are linked to an SPI node.
 */
struct _hwspi
{
        reg8_t spcr; //!< Control register.
        reg8_t spsr; //!< Status register.
        reg8_t spdr; //!< SPI data transfer register.
} __PACK__;

#ifdef __cplusplus
}
#endif
#endif /* __SPIBUS328_H_ */