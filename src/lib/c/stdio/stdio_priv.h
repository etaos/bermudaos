/*
 *  BermudaOS - StdIO - I/O private header
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

#ifndef __STDIO_PRIV_H
#define __STDIO_PRIV_H

#define BUFF 16

#define FLT_DIGITS 2

extern int convert_to_num(uint32_t num, uint8_t base, bool sign, 
							  bool caps, FILE *stream);
extern void print_flt(double num, FILE *output);

#endif /* __STDIO_PRIV_H */