/***************************************************************************
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
 *****************************************************************************/
 
#ifdef _DEBUG

#ifndef __SWAPS_H__
#define __SWAPS_H__

#include "HeapCheckPrototypes.h"

#ifdef PRE_CHECK
#define malloc(x)	HeapCheckPreFenceMalloc(x)
#define calloc(x,y)	HeapCheckPreFenceCalloc(x*y)
#define realloc(x,y)	HeapCheckPreFenceRealloc(x,y)
#define free(x)		HeapCheckPreFenceFree(x)
#else
#define malloc(x)	HeapCheckPostFenceMalloc(x)
#define calloc(x,y)	HeapCheckPostFenceCalloc(x*y)
#define realloc(x,y)	HeapCheckPostFenceRealloc(x,y)
#define free(x)		HeapCheckPostFenceFree(x)
#endif

#endif /* __SWAPS_H__ */

#endif /* _DEBUG */
