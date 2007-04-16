/*  
 *  HeapCheck - a heap debugging library
 *  Copyright (C) 2001  Thanassis Tsiodras (ttsiod@softlab.ntua.gr)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef __cplusplus

// We only exist in debug builds...
#ifdef _DEBUG

#include <windows.h>
#include <stdio.h>

#include "HeapCheckPrototypes.h"

// Checks for underuns...
#ifdef PRE_CHECK

void *operator new(size_t size) 
{   
    return HeapCheckPreFenceMalloc(size); 
}

void *operator new[](size_t size) 
{
    return HeapCheckPreFenceMalloc(size); 
}

void operator delete(void *p) 
{
    if (p)
	HeapCheckPreFenceFree(p);
}

void operator delete[](void *p) 
{
    if (p)
	HeapCheckPreFenceFree(p); 
}

#else 

// Checks for overuns...
void *operator new(size_t size) 
{        
    return HeapCheckPostFenceMalloc(size); 
}

void *operator new[](size_t size) 
{
    return HeapCheckPostFenceMalloc(size); 
}

void operator delete(void *p) 
{
    if (p)
	HeapCheckPostFenceFree(p);
}

void operator delete[](void *p) 
{
    if (p)
	HeapCheckPostFenceFree(p); 
}

#endif /* PRE_CHECK */

#endif /* _DEBUG */

#endif /* __cplusplus */
