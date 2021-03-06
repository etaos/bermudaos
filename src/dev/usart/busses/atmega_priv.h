/*
 *  BermudaOS - ATmega USART bus controller
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

#ifndef __ATMEGA_USART_H
#define __ATMEGA_USART_H

#include <arch/avr/io.h>

#ifndef BAUD
#define BAUD 9600
#warning "Serial baudrate has not been defined, defaulting to 9600."
#endif

	/* baud calculations */
#undef USE_2X

	/* Baud rate tolerance is 2 % unless previously defined */
#ifndef BAUD_TOL
#  define BAUD_TOL 2
#endif

#ifdef __ASSEMBLER__
#define UBRR_VALUE (((F_CPU) + 8 * (BAUD)) / (16 * (BAUD)) -1)
#else
#define UBRR_VALUE (((F_CPU) + 8UL * (BAUD)) / (16UL * (BAUD)) -1UL)
#endif

#if 100 * (F_CPU) > \
	(16 * ((UBRR_VALUE) + 1)) * (100 * (BAUD) + (BAUD) * (BAUD_TOL))
#  define USE_2X 1
#elif 100 * (F_CPU) < \
	(16 * ((UBRR_VALUE) + 1)) * (100 * (BAUD) - (BAUD) * (BAUD_TOL))
#  define USE_2X 1
#else
#  define USE_2X 0
#endif

#if USE_2X
	/* U2X required, recalculate */
#undef UBRR_VALUE

#ifdef __ASSEMBLER__
#define UBRR_VALUE (((F_CPU) + 4 * (BAUD)) / (8 * (BAUD)) -1)
#else
#define UBRR_VALUE (((F_CPU) + 4UL * (BAUD)) / (8UL * (BAUD)) -1UL)
#endif

#if 100 * (F_CPU) > \
	(8 * ((UBRR_VALUE) + 1)) * (100 * (BAUD) + (BAUD) * (BAUD_TOL))
#  warning "Baud rate achieved is higher than allowed"
#endif

#if 100 * (F_CPU) < \
	(8 * ((UBRR_VALUE) + 1)) * (100 * (BAUD) - (BAUD) * (BAUD_TOL))
#  warning "Baud rate achieved is lower than allowed"
#endif

#endif /* USE_U2X */

#ifdef UBRR_VALUE
#  define UBRR0L_VALUE (UBRR_VALUE & 0xff)
#  define UBRR0H_VALUE (UBRR_VALUE >> 8)
#endif

#define UBRR0L MEM_IO8(0xC4)
#define UBRR0H MEM_IO8(0xC5)
#define UDR0 MEM_IO8(0xC6)

#define UCSR0A MEM_IO8(0xC0)
#define UCSR0B MEM_IO8(0xC1)
#define UCSR0C MEM_IO8(0xC2)

#define U2X0 1
#define UCSZ00 1
#define UCSZ01 2
#define TXEN0 3
#define RXEN0 4
#define UDRE0 5
#define TXCn 6
#define RXC0 7

// interrupts
#define RXCIE0 7
#define TXCIE0 6
#define UDRIE0 5

struct hw_uart
{
	reg8_t ucsra; //!< UART control & status register 0-a.
	reg8_t ucsrb; //!< UART control & status register 0-b.
	reg8_t ucsrc; //!< UART control & status register 0-c.
	reg8_t ubrrl; //!< UART bit rate register 0 LSB.
	reg8_t ubrrh; //!< UART bit reate register 0 MSB.
	reg8_t udr;   //!< UART data register 0.
} __attribute__((packed));

typedef struct hw_uart HW_USART;

#endif /* __ATMEGA_USART_H */