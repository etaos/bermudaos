/*
 *  BermudaOS - IRQ header
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
 * \file include/arch/irq.h IRQ header.
 * \brief Definitions IRQ data structures.
 */

#ifndef __IRQ_DATA_H_
#define __IRQ_DATA_H_

struct irq_data {
	struct irq_data *next;
	
	unsigned int irq;
	uint8_t vector;
	
	void *data;
};

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
	#include <arch/avr/interrupts.h>
#endif

/**
 * \def ISR
 * \brief Define an Interrupt Service Handler.
 * \param __fname Name of the ISR.
 * \param __arg Name of the argument.
 * \param __type Type of the argument <i>__arg</i>.
 */
#define ISR(__fname, __arg, __type) \
	void __fname(typeof (__type) (__arg)) __attribute__((ISR_HANDLE_ATTRIBS)); \
	void __fname(typeof (__type) (__arg))

/**
 * \def ISR_DEF
 * \brief Define an ISR. Useful for header files.
 * \param __fname Name of the ISR.
 * \param __arg Name of the argument.
 * \param __type Type of the argument <i>__arg</i>.
 */
#define ISR_DEF(__fname, __arg, __type) \
	extern void __fname(typeof (__type) (__arg)) __attribute__((ISR_HANDLE_ATTRIBS))

#endif