/*
 *  BermudaOS - Virtual timers
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

/** \file virt_timer.c */

#include <bermuda.h>
#include <sys/virt_timer.h>
#include <arch/io.h>

/**
 * \addtogroup vtimers Timer Management API
 * \brief Timer management API.
 * 
 * API to handle timers which run in the back ground. An example of the use of
 * these timers is in the BermudaThreadSleep function. The scheduler uses them
 * to clock sleep time's.
 * 
 * @{
 */

/**
 * \var BermudaTimerList
 * \brief Timer linked list.
 * \see BermudaTimerAdd
 * \private
 * 
 * Linked list of timer objects. The list is in control of the timer module.
 */
PRIVATE WEAK VTIMER *BermudaTimerList = NULL;

/**
 * \var delay_loop_count
 * \brief Delay loops.
 * \see BermudaTimerInit
 * \private
 * 
 * Amount of loops per system tick.
 */
PRIVATE WEAK unsigned long delay_loop_count = 0;

/**
 * \brief Initialise the timer module.
 * \note Called by the BermudaOS initialisation sequence.
 * \warning Should never be called by any user application.
 * \see delay_loop_count
 * 
 * Registrates and starts a hardware timer. Also the delay_loop_count will
 * be intialised.
 */
PUBLIC void BermudaTimerInit()
{
        BermudaInterruptsSave();
        sei();
        
        unsigned long count = BermudaTimerGetTickCount();
        while(count == BermudaTimerGetTickCount())
                delay_loop_count++;
        
        BermudaInterruptsRestore();
        /*
         * The loop above takes more cycles due to the function call in the
         * while loop. So a correction has to be applied.
         */
#ifdef __AVR__
        delay_loop_count *= 103UL;
        delay_loop_count /= 26UL;
#else
        delay_loop_count *= 137UL;
        delay_loop_count /= 25UL;
#endif
}

/**
 * \brief Delay for given amount of micro seconds.
 * \param us Amount of micro seconds to delay.
 * \todo Create a NOP definition for arch independency.
 * 
 * The CPU will busy wait for the given amount of micro seconds.
 */
PUBLIC void BermudaDelay_us(unsigned long us)
{
        register unsigned long count = delay_loop_count * us / 1000;
        
        while(count--)
                __asm__ __volatile__("mov r0, r0");
}

/**
 * \brief Delay for given amount of mili seconds.
 * \param us Amount of mili seconds to delay.
 * 
 * The CPU will busy wait for the given amount of mili seconds.
 */
PUBLIC void BermudaDelay(unsigned char ms)
{
        BermudaDelay_us((unsigned long)ms * 1000);
}

/**
 * \brief Create a new timer.
 * \param ms Time until it will fire in milli seconds.
 * \param fn Fire function.
 * \param arg Argument passed to <b>fn</b>.
 * \param flags Can be set to either BERMUDA_PERIODIC or BERMUDA_ONE_SHOT
 * \return The created timer object.
 * \see BERMUDA_ONE_SHOT
 * \see BERMUDA_PERIODIC
 * \see BermudaSchedulerExec
 * \note Each time BermudaSchedulerExec is called the timer list is processed.
 *       If threads are not available, they will be handled in the timer interrupt.
 * 
 * Create's a new timer object based on the given parameters. When the timer
 * expires it will call the given function <b>fn</b>.
 */
PUBLIC VTIMER *BermudaTimerCreate(unsigned int ms, vtimer_callback fn, void *arg,
                                     unsigned char flags)
{
        VTIMER *timer = BermudaHeapAlloc(sizeof(*timer));
        if(NULL == timer)
                return NULL;
                
        timer->handle = fn;
        timer->arg = arg;
        timer->flags = flags;
        BermudaTimerAdd(timer);
        return timer;
}

/**
 * \brief Add the given timer to the list.
 * \param timer New timer to add.
 * \warning Be careful not to add a timer twice.
 * \see _vtimer
 * \private
 * \todo Fix head == NULL cases
 * 
 * The timer will be added to the list of timers. The list is ordered by the
 * <b>ticks_left</b> field.
 */
PRIVATE WEAK void BermudaTimerAdd(VTIMER *timer)
{
        VTIMER **vtpp = &BermudaTimerList, *vtp = BermudaTimerList;
        
        foreach(vtp, vtp)
        {
                if(timer->ticks_left < vtp->ticks_left)
                {
                        timer->next = vtp;
                        *vtpp = timer;
                        vtp->ticks_left -= timer->ticks_left;
                        break;
                }
                vtpp = &vtp->next;
        }
        if(timer->next)
                timer->ticks_left -= vtp->ticks_left;
}

/**
 * \brief Delete a timer.
 * \param timer Timer to delete.
 * \see BermudaTimerProcess
 * 
 * When a timer expired or is not needed anymore, it can be stopped and deleted
 * by this function.
 */
void BermudaTimerExit(VTIMER *timer)
{
}

/**
 * \fn BermudaTimerProcess()
 * \brief Process all virtual timers.
 * 
 * Update all virtual timers. This action is done before switching context by
 * default.
 */
void BermudaTimerProcess()
{
}

// @}
