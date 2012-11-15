/*
 *  BermudaNet - VLAN data structure header.
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
 * \file include/tokenbucket.h Token bucket algorithm header.
 * \addtogroup net
 * @{
 * \addtogroup tbucket Token bucket
 * @{
 */

#ifndef __TOKENBUCKET_H
#define __TOKENBUCKET_H

#include <stdlib.h>

#include <net/netbuff.h>

struct netbuff;

/**
 * \brief Traffic can be shared equally using a token bucket.
 */
struct tbucket
{
	volatile struct netbuff_queue *queue; //!< Queue to enqueue packets.
	
	uint64_t rate; //!< Maximum rate of the uplink.
	uint64_t tokens; //!< Amount of tokens, which can be used to 'buy' packets.
	bool active;
} __attribute__((packed));

#ifdef __DOXYGEN__
#else
__DECL
#endif
extern void pay_packet(struct tbucket *bucket, struct netbuff *packet);
extern bool tbucket_can_afford_packet(struct tbucket *bucket, struct netbuff *packet);
extern int cash_tokens(struct tbucket *bucket, size_t tokens);
#ifdef __DOXYGEN__
#else
__DECL_END
#endif

#endif /* __TOKENBUCKET_H */

//@}
//@}