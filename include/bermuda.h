/*
 *  BermudaOS - BermudaOS stdlib
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
 * \file bermuda.h 
 * \brief BermudaOS standard library header.
 * 
 * This header contains all basic defines, functions and types.
 */

#ifndef __BERMUDA_H
#define __BERMUDA_H

#ifndef BERMUDANET_CONFIG
#include <config.h>
#endif

#include <arch/types.h>

#include <sys/out.h>

#ifdef __cplusplus
#include <cplusplus.h>

#define __DECL extern "C" {
#define __DECL_END }

#else /* !__cplusplus */

#define __DECL
#define __DECL_END

#endif /* __cplusplus */

#define E_SUCCESS 0
#define E_GENERIC 1
#define E_TIMEOUT 2

#define FALSE 0
#define TRUE !FALSE

#define false FALSE
#define true TRUE

#define POINTER(T) (typeof(T*))
#define ARRAY(T, N) (typeof(T[N]))

#define NEXT(E) ((typeof(E))((E)->next))

#ifndef NULL
#define NULL (void*)0
#endif

/**
 * \def __PACK__
 * \brief Packed attribute.
 * 
 * Used to counter compiler optimizations on structures.
 */
#define __PACK__ __attribute__((packed))

/**
 * \def PRIVATE
 * \brief Hidden visibility.
 * 
 * The function is not visible outside of its own compile unit.
 */
#define PRIVATE __attribute__ ((visibility ("hidden")))

/**
 * \def WEAK
 * \brief Declare a function weak.
 * 
 * Weak functions are not marked as global functions. Same as <i>'static'</i>.
 */
#define WEAK    __attribute__((weak))

/**
 * \brief GCC will try to optimize the function where this attribute is passed to.
 */
#define __maxoptimize __attribute__((optimize("O3")))

/**
 * \def PUBLIC
 * \brief Declare a function public.
 * 
 * Function declared public are explicitly marked visible, so other compile units
 * can access the function.
 */
#define PUBLIC  __attribute__((externally_visible))

#ifndef __cplusplus
/**
 * \def public
 * \brief Declare a function public.
 * \note This define is only defined if __cplusplus is <b>NOT</b> defined (i.e. not programming in C++).
 *
 * Function declared public are explicitly marked visible, so other compile units
 * can access the function.
 */
#define public PUBLIC
#endif

/**
 * \def __raw
 * \brief Raw function.
 * 
 * Raw function have no epilogue or prologue. The programmer has to provide them.
 * This can be useful in ISR's.
 */
#define __raw   __attribute__((naked))

/**
 * \def signal
 * \brief ISR
 * 
 * Declare a function as an ISR.
 */
#define __sig   __attribute__((signal))

/**
 * \def __link
 * \brief Always link function.
 * 
 * Functions marked with <b>__link</b> are always linked into the
 * executable, even if they <b>might</b> appear as unused to the linker.
 */
#define __link __attribute__((used))

/**
 * \def __noinline
 * \brief Prevent the compiler from optimizing function calls.
 */
#define __noinline __attribute__((noinline))

/**
 * \def __force_inline
 * \brief Make sure a function is inline.
 * 
 * The function is treated as an inline function, even if no form of optimization is used.
 */
#define __force_inline __attribute__((always_inline))

/**
 * \typedef mutex_t
 * \brief Mutual exclusion type.
 */
typedef volatile unsigned char mutex_t;

typedef volatile unsigned char*   reg8_t;
typedef volatile unsigned short*  reg16_t;
typedef volatile uint32_t*        reg32_t;
typedef unsigned char             bool;


#ifdef __cplusplus
extern "C" {
#endif

extern int BermudaInit();

#ifdef __cplusplus
}
#endif

#include <sys/mem.h>
#include <lib/list/list.h>

#endif
