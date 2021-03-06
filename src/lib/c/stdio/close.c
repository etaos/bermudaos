/*
 *  BermudaOS - StdIO - I/O close
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
#include <stdio.h>

/**
 * \file src/lib/c/stdio/close.c stdio close functions
 */

/**
 * \brief StdIO close function.
 * \param fd File descriptor to close.
 */
PUBLIC int close(int fd)
{
	int rc = 0;
	
	if(!__iob[fd]) {
		return -1;
	}
	
	if(__iob[fd]->close != NULL) {
		rc = __iob[fd]->close(__iob[fd]);
	}
	
	__iob[fd]->fd = -1;
	__iob[fd] = NULL;
	return rc;
}

