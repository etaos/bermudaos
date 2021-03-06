/*
 *  BermudaNet - Token bucket library
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

#include <net/netbuff.h>
#include <net/tokenbucket.h>

/**
 * \file src/net/tokenbucket.c
 * \brief Bitrate supression.
 * 
 * \addtogroup net
 * @{
 * \addtogroup tbucket Token bucket
 * \brief Tokenbucket bitrate supression.
 * With a tokenbucket, the maximum transfer rate (i.e. bitrate) of the connection can be limited.
 * 
 * \section algo Algorithm
 * The concept of the algorithm is as follows: \n
 * * Let \f$R\f$ be the bitrate, \f$ t \f$ the amount of tokens added and \f$ n \f$ the amount 
 * of tokens needed.\n
 * \f$ f(t) = 1/R \f$ \n
 * * The bucket can have at the most \f$ B \f$ tokens. If a token arrives when the bucket is full, it is
 * discarded. \n
 * * When a network layer PDU arrives of \f$ n \f$ bytes, \f$ n \f$ bytes are removed from the bucket, and the
 * PDU is sent to the network driver. \n
 * * If viewer than \f$ n \f$ tokens are available, the packet will be queued.
 * @{
 */

/**
 * \brief Add tokens to the token bucket.
 * \param bucket Token bucket to add the tokens to.
 * \param tokens Amount of tokens to add.
 * \return 0 if the tokens are added successfully, 1 the given amount of tokens would cause a roll
 *         over.
 */
PUBLIC int cash_tokens(struct tbucket *bucket, size_t tokens)
{
	/*
	 * Don't roll over!
	 */
	if((bucket->tokens + tokens) < bucket->tokens) {
		bucket->tokens = 0;
		bucket->tokens = ~(bucket->tokens);
		return 1;
	} else {
		bucket->tokens += tokens;
		return 0;
	}
}

/**
 * \brief Determines wether a packet can be afforded yes or no.
 * \param bucket Bucket with the tokens.
 * \param packet Packet to check against <i>bucket</i>.
 * \return <i>TRUE</i>, if the packet can be afforded, <i>FALSE</i> otherwise.
 */
PUBLIC bool tbucket_can_afford_packet(struct tbucket *bucket, struct netbuff *packet)
{
	if(bucket->tokens < packet->length) {
		return false;
	} else {
		return true;
	}
}

//@}
//@}
