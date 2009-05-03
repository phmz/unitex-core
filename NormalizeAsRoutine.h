 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 2.1 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Lesser General Public License for more details.
  * 
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
  *
  */

#ifndef NormalizeAsRoutineH
#define NormalizeAsRoutineH


#define MAX_TAG_LENGTH 4000
#define KEEP_CARRIAGE_RETURN 0
#define REMOVE_CARRIAGE_RETURN 1
#define BUFFER_SIZE 1000000
/* When we are at less than 'MARGIN_BEFORE_BUFFER_END' from the end of the buffer,
 * we will refill it, unless we are at the end of the input file. */
#define MARGIN_BEFORE_BUFFER_END (MAX_TAG_LENGTH+1000)

int normalize(char*, char*, int, char*);

#endif
