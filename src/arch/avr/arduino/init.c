/*
 *  BermudaOS - External signals
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

#include <stdlib.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#include <arch/avr/chip/uart.h>
#include <arch/avr/io.h>
#include <arch/avr/timer.h>

#define LED_DDR  BermudaGetDDRB()
#define LED_PORT BermudaGetPORTB()
#define LED_PIN  BermudaGetPINB()
#define LED      PINB5

void flash_led(uint8_t count)
{
        while (count--)
        {
                LED_PORT |= _BV(LED);
                _delay_ms(100);
                LED_PORT &= ~_BV(LED);
                _delay_ms(100);
        }
}

int main(void)
{
        BermudaInitUART();
        char x[128];
        LED_DDR = 0xFF;
        spb(LED_PORT, LED);
        sprintf(x, "Value of PORTB: %x\n", BermudaGetPORTB());
        printf(x);
        printf("Address of PORTB: %p\n", BermudaGetAddressPORTB());
        BermudaInitTimer0();
        sei();
        while(1)
        {
                printf("Timer count: %x\n", (uint16_t)BermudaGetTimerCount());
                _delay_ms(1000);
        }
        return 0;
}