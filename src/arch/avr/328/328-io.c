/*
 *  BermudaOS - Pin I/O
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

#include <bermuda.h>

#include <arch/io.h>
#include <arch/avr/io.h>

const struct avr_io io =
{
        .portb          = BermudaGetAddressPORTB(),
        .portc          = BermudaGetAddressPORTC(),
        .portd          = BermudaGetAddressPORTD(),
        
        .pinb           = BermudaGetAddressPINB(),
        .pinc           = BermudaGetAddressPINC(),
        .pind           = BermudaGetAddressPIND(),
        
        .ddrb           = BermudaGetAddressDDRB(),
        .ddrc           = BermudaGetAddressDDRC(),
        .ddrd           = BermudaGetAddressDDRD(),
        .sreg           = BermudaGetAddressSREG(),
};

const unsigned short ROM BermudaPortToOutput[] =
{
        (unsigned short)PIN_NOT_AVAILABLE,
        (unsigned short)BermudaGetAddressPORTB(),
        (unsigned short)BermudaGetAddressPORTC(),
        (unsigned short)BermudaGetAddressPORTD(),
};

const unsigned short ROM BermudaPortToInput[] =
{
        (unsigned short)PIN_NOT_AVAILABLE,
        (unsigned short)BermudaGetAddressPINB(),
        (unsigned short)BermudaGetAddressPINC(),
        (unsigned short)BermudaGetAddressPIND(),
};

const unsigned short ROM BermudaPortToMode[] =
{
        (unsigned short)PIN_NOT_AVAILABLE,
        (unsigned short)BermudaGetAddressDDRB(),
        (unsigned short)BermudaGetAddressDDRC(),
        (unsigned short)BermudaGetAddressDDRD(),
};

inline void BermudaSafeCli(unsigned char *ints)
{
        *ints = *(AvrIO->sreg) & 0x80;
        __asm__ __volatile__ ("cli" ::);
}
