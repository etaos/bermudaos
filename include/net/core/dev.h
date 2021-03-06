/*
 *  BermudaNet - Device agnostic header
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

#ifndef __DEV_AGN_H
#define __DEV_AGN_H

#include <stdlib.h>

#include <net/netdev.h>
#include <net/netbuff.h>

#ifdef __DOXYGEN__
#else
__DECL
#endif
extern int netif_init(struct netdev *dev);
extern int netif_init_dev(struct netdev *dev);
#ifdef __DOXYGEN__
#else
__DECL_END
#endif
#endif /* __DEV_AGN_H */