/*
 *  BermudaOS - StdLib
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

//!< \file include/stdlib.h Standard library header.

#ifndef __STDLIB_HEADER_
#define __STDLIB_HEADER_

#include <bermuda.h>


__DECL
extern int memcmp(const void *s1, const void *s2, size_t n);
extern void _exit();
__DECL_END

#endif /* __STDLIB_HEADER_ */