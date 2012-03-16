/*
 *  BermudaOS - Threading module
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

#ifndef __THREAD_H
#define __THREAD_H

#include <inttypes.h>

#ifndef RTSCHED

/**
  * \struct struct thread
  * \brief Describes the state of a thread
  */
struct thread
{
        struct thread *next;
        struct thread *prev;
        void *stack;            /* start of the stack */
        unsigned char *sp;      /* stack pointer */
        void *param;            /* thread parameter */
        unsigned char prio;
        unsigned short stack_size;
        unsigned int sleep_time;
} __attribute__((packed));
typedef struct thread THREAD;
#endif

#endif