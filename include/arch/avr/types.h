/*
 *  BermudaOS - Arch specific types.
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

#ifndef __ARCH_AVR_TYPES_H_
#define __ARCH_AVR_TYPES_H_

#include <bermuda.h>

#define __byte_swap2(val) \
	((((val) & 0xff) << 8) |        \
	(((val) & 0xff00) >> 8))

#define __byte_swap4(val)			\
	((((val) & 0xff) << 24) |			\
	(((val) & 0xff00) << 8) |		\
	(((val) & 0xff0000) >> 8) |	\
	(((val) & 0xff000000) >> 24))

typedef unsigned long long int uint64_t;
typedef unsigned long int uint32_t;
typedef unsigned int uint16_t;
typedef unsigned char uint8_t;
typedef unsigned int uptr;
typedef unsigned int size_t;
typedef long long time_t;
typedef signed long int32_t;

typedef uint32_t __32be;
typedef uint16_t __16be;
#endif /* __AVR_TYPES_H */