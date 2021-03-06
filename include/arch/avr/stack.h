/*
 *  BermudaOS - Stack support :: Used by the general schedule module
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

/** \file include/arch/avr/stack.h */

#ifndef __STACK_H
#define __STACK_H

#include <bermuda.h>
#include <sys/thread.h>
#include <arch/avr/io.h>

#define STACK_L SFR_IO8(0x3D)
#define STACK_H SFR_IO8(0x3E)
#define STACK   ((STACK_L) | ((STACK_H) << 8))

typedef unsigned char* stack_t;

__DECL
extern void BermudaStackInit(THREAD *t, stack_t stack, 
                             unsigned short stack_size, thread_handle_t handle);
extern void BermudaStackFree(THREAD *t);
extern void BermudaStackSave(stack_t stack);
__DECL_END

#endif
