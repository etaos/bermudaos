/*
 *  BermudaOS - ATmega SPI driver
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

#ifndef __SPI_328_H
#define __SPI_328_H

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
 * \def SPI_CTRL
 * \brief SPI control register
 */
#define SPI_CTRL    SFR_IO8(0x2C)
#define SPCR SPI_CTRL //!< SPCR register

/**
 * \def SPI_STATUS
 * \brief SPI statud register
 */
#define SPI_STATUS  SFR_IO8(0x2D)
#define SPSR SPI_STATUS //!< SPSR register

/**
 * \def SPI_DATA
 * \brief SPI data register
 */
#define SPI_DATA    SFR_IO8(0x2E)
#define SPDR SPI_DATA //!<->SPDR register

/**
 * \def SPI_MAX_DIV
 * \brief Maximum SPI clock divider for the AVR 328.
 */
#define SPI_MAX_DIV 128

/**
 * \def SPI_MODE_SHIFT
 * \brief Bit location of the SPI mode bits.
 */
#define SPI_MODE_SHIFT 2

/**
 * \def SPI_SCK
 * \brief SCK pin.
 */
#define SPI_SCK  BIT(5)

/**
 * \def SPI_MISO
 * \brief MISO pin.
 */
#define SPI_MISO BIT(4)

/**
 * \def SPI_MOSI
 * \brief MOSI pin.
 */
#define SPI_MOSI BIT(3)

/**
 * \def SPI_SS
 * \brief SS pin.
 */
#define SPI_SS  BIT(2)

/**
 * \def SPI_MAX_PRES
 * \brief Lowest possible prescaler.
 * \note SPI_DEFAULT_PRES is suggested.
 * \see SPI_DEFAULT_PRES
 */
#define SPI_MAX_PRES 128

/**
 * \def SPI_DEFAULT_PRES
 * \brief Suggested prescaler.
 */
#define SPI_DEFAULT_PRES 16

/**
 * \def SPI_MASTER_ENABLE
 * \brief Bit location of the SPI master enable bit in the SPCR
 */
#define SPI_MASTER_ENABLE BIT(4)

/**
 * \def SPI_IRQ_ENABLE
 * \brief Bit to enable the SPI IRQ.
 * \note The IE bit in the SREG register has to be set.
 */
#define SPI_IRQ_ENABLE BIT(7)

/**
 * \def SPI_ENABLE
 * \brief Bit to enable the SPI globally.
 */
#define SPI_ENABLE BIT(6)

#define SPR0 BIT(0)
#define SPR1 BIT(1)
#define SPIF 7

#define SPI_FRQ(__div) (F_CPU / __div)


#endif
