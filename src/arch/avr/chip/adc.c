/*
 *  BermudaOS - Analog Digital Converter
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

#include <avr/io.h>
#include <arch/avr/chip/adc.h>

static struct adc *BermADC;

static inline struct adc *BermGetADC()
{
        return BermADC;
}

static void BermADCUpdate(adc)
struct adc* adc;
{
        adc->adcl = ADCL;
        adc->adch = ADCH;
        adc->admux = ADMUX;
        adc->adcsra = ADCSRA;
        adc->adcsrb = ADCSRB;
        adc->didr0 = DIDR0;
        return;
}
