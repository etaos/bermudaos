/*
 *  BermudaOS - StdIO - I/O open
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
#include <string.h>

/**
 * \brief Open a file.
 * \param fname Name of the file to open.
 * \param mode File mode to use.
 * \return The file descriptor.
 */
PUBLIC int fdopen(char *fname, unsigned char mode)
{
	int i = 0;
	FILE *c = 0;
	
	for(; c; c = c->next) {
		if(!strcmp(c->name, fname)) {
			for(i = 3; i < MAX_OPEN; i++) {
				if(__iob[i] == NULL) {
					__iob[i] = c;
					return i; // file is opened
				} else if(!strcmp(__iob[i]->name, fname)) {
					return i; // file is already open
				}
			}
		}
		if(c->next == NULL) {
			break;
		}
	}
	
	return -1;
}
