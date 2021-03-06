/*
 * Copyright (C) 2001-2006 by egnite Software GmbH. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY EGNITE SOFTWARE GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL EGNITE
 * SOFTWARE GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 *  Aditional license:
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

/** \file event.c */

#include <bermuda.h>

#include <arch/io.h>

#include <sys/sched.h>
#include <sys/thread.h>
#include <sys/events/event.h>

#ifndef __THREADS__
#       error Handling of events is not possible without threading.
#endif

/**
 * \addtogroup event_management Event Management API
 * \brief Thread synchronization principles.
 * 
 * A thread may wait for a certain event by using the function BermudaEventWait
 * . Another thread can wakeup the event by using the function BermudaEventSignal. \n
 * 
 * The function BermudaEventPost may <b>NEVER</b> used from interrupt context. When
 * an event has to be posted from interrupt context, one can use the function
 * BermudaEventSignalFromISR. \n
 * \n
 * If an event is posted, the signaled thread takes over the CPU (if its priority
 * is higher or equal to the one running).\n
 * \n
 * An event queue needs the following definition:
 * \code{.c}
 * volatile THREAD *QueueName;
 * \endcode
 * 
 * @{
 */

/**
 * \fn BermudaEventWait(volatile THREAD **tqpp, unsigned int tmo)
 * \brief Wait for an event.
 * \param queue Wait in this queue.
 * \param tmo <i>Time out</i>. Maximum time to wait.
 * \see BermudaEventSignal
 * \see BermudaEventSignalFromISR
 * 
 * Wait for an event in a specific time for a given amount of time. If you
 * want to wait infinite use <i>BERMUDA_EVENT_WAIT_INFINITE</i>.
 */
PUBLIC int BermudaEventWait(volatile THREAD **tqpp, unsigned int tmo)
{
	volatile THREAD *tqp;

	BermudaEnterCritical();
	tqp = *tqpp;
	BermudaExitCritical();
        
	if(tqp == SIGNALED) { // queue is empty
		BermudaEnterCritical();
		*tqpp = 0;
		BermudaExitCritical();
                
		BermudaThreadYield(); // give other threads a chance
		return 0;
	}
        
	/*
	 * The queue is currently locked, this means the current thread has
	 * to wait. Remove it from the running queue and add it to the event
	 * queue.
	 */
	BermudaThreadQueueRemove(&BermudaRunQueue, BermudaCurrentThread);
	BermudaThreadPrioQueueAdd((THREAD**)tqpp, BermudaCurrentThread);
	BermudaCurrentThread->state = THREAD_SLEEPING;
        
	if(tmo) {
		BermudaCurrentThread->th_timer = BermudaTimerCreate(tmo, &BermudaEventTMO,
		                                                   (void*)tqpp,
		                                                   BERMUDA_ONE_SHOT
		);
	}
	else {
		BermudaCurrentThread->th_timer = NULL;
	}
	BermudaSchedulerExec(); // DO NOT USE THREAD YIELDING! -> queue's are editted
        
	// When the thread returns
	if(BermudaCurrentThread->th_timer == SIGNALED) { // event timed out
		BermudaCurrentThread->th_timer = NULL;
		return -1;
	}
        
	return 0; // event posted succesfuly
}

/**
 * \brief Wait for the next event.
 * \param tqpp Waiting queue.
 * \param tmo Event waiting time-out.
 * 
 * Waits for the next upcoming event, even if the waiting queue is empty
 * (SIGNALED).
 */
PUBLIC int BermudaEventWaitNext(volatile THREAD **tqpp, unsigned int tmo)
{	
	if(tqpp == NULL) {
		return -1;
	}
	
	BermudaEnterCritical();
	if(*tqpp == SIGNALED) {
		*tqpp = NULL;
	}
	BermudaExitCritical();
	return BermudaEventWait(tqpp, tmo);
}

/**
 * \fn BermudaEventTMO(VTIMER *timer, void *arg)
 * \brief Timeout function.
 * \param timer Timer object which called this function.
 * \param arg Void casted argument of the event queue, where a thread received a
 * timeout in.
 * 
 * When a thread timeouts waiting for an event, the wait function will return with
 * an error.
 */
PRIVATE WEAK void BermudaEventTMO(VTIMER *timer, void *arg)
{
	THREAD *volatile *tqpp, *tqp;
        
	tqpp = (THREAD**)arg;
	BermudaEnterCritical();
	tqp = *tqpp;
	BermudaExitCritical();
        
	if(tqp != SIGNALED) {
		while(tqp) {
			if(tqp->th_timer == timer) { 
			// found the timed out thread
				BermudaEnterCritical();
				*tqpp = tqp->next;
				if(tqp->ec) {
					if(tqp->next) {
						tqp->next->ec = tqp->ec;
					}
					else {
						*tqpp = SIGNALED;
					}
					tqp->ec = 0;
				}
				BermudaExitCritical();
                                
				// mark thread as ready to go
				tqp->state = THREAD_READY;
				BermudaThreadPrioQueueAdd(&BermudaRunQueue, tqp);
				tqp->th_timer = SIGNALED; // waiting thread can handle
										  // on this signal
				break;
            }
			tqpp = &tqp->next;
			tqp = tqp->next;
		}
	}
}

/**
 * \brief Post an event.
 * \param tqpp Queue to post an event to.
 * \see BermudaEventWait
 * 
 * The given queue will be signaled and the thread with the highest priority will
 * become runnable again.
 */
PUBLIC int BermudaEventSignalRaw(THREAD *volatile*tqpp)
{
	THREAD *t;
	BermudaEnterCritical();
	t = *tqpp;
	BermudaExitCritical();
	int rc = -1;
        
	if(t != SIGNALED) {
		if(t) {
			BermudaEnterCritical();
			*tqpp = t->next;
			if(t->ec) {
				if(t->next)
					t->next->ec = t->ec;
				else
					*tqpp = SIGNALED;

				t->ec = 0;
			}
			BermudaExitCritical();
                        
			if(t->th_timer) {
				BermudaTimerStop(t->th_timer);
				t->th_timer = NULL;
			}
                        
			t->state = THREAD_READY;
			BermudaThreadPrioQueueAdd(&BermudaRunQueue, t);
                        
			rc = 0; // post done succesfuly
		}
		else {
			BermudaEnterCritical();
			*tqpp = SIGNALED;
			BermudaExitCritical();
			rc = 0;
		}
    }
	return rc; // could not post
}

/**
 * \brief Post an event.
 * \param tqpp Queue to post an event to.
 * \see BermudaEventWait
 * 
 * The given queue will be signaled and the thread with the highest priority will
 * become runnable again. The signaled thread will run if it has an equal or
 * higher priority than the currently running thread.
 */
PUBLIC int BermudaEventSignal(volatile THREAD **tqpp)
{
        int ret = BermudaEventSignalRaw((THREAD*volatile*)tqpp);
        BermudaThreadYield();
        return ret;
}

/**
 * \brief Post an event from an ISR.
 * \param tqpp Event queue.
 * \see BermudaEventSignal
 * 
 * Should only be used when an event has to be posted from an ISR. If you are in
 * a normal context, BermudaEventSignal should be used.
 */
PUBLIC void BermudaEventSignalFromISR(volatile THREAD **tqpp)
{
	if(*tqpp == NULL) {
		*tqpp = SIGNALED;
	}
	else if(*tqpp != SIGNALED) {
		(*tqpp)->ec++;
	}
}

// @}

