/*
 *  BermudaOS - fputc
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

PUBLIC int fputc(int c, FILE *file)
{
	int rc = -1;
	if((file->flags & __SWR) != 0) {
		rc = file->put(c, file);
		if(rc != -EOF || rc == c) {
			file->length++;
		}
	}
	return rc;
}

PUBLIC int fputs(char *s, FILE *f)
{
	for (; *s; s++) {
		fputc(*s, f);
	}
	
	return 0;
}
