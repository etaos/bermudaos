/*
 *  BermudaOS - printf
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

//! \file src/lib/c/stdio/printf_p.c Text output module.

#ifdef __AVR__

#include <stdlib.h>
#include <stdio.h>
#include <arch/avr/pgm.h>

PUBLIC int printf_P(const prog_char * fmt, ...)
{
	int i;

	va_list va;
	va_start(va, fmt);
	i = vfprintf_P(stdout, fmt, va);
	va_end(va);

	return i;
}
#endif
