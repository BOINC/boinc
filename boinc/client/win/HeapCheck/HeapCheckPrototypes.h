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
 
#ifndef __PROTOTYP_H__
#define __PROTOTYP_H__

#ifdef __cplusplus

extern "C" {

#endif /* __cplusplus */

void *HeapCheckPostFenceMalloc(size_t s);
void *HeapCheckPreFenceMalloc(size_t s);
void *HeapCheckPostFenceCalloc(size_t s);
void *HeapCheckPreFenceCalloc(size_t s);
void *HeapCheckPostFenceRealloc(void *p, size_t s);
void *HeapCheckPreFenceRealloc(void *p, size_t s);
void HeapCheckPostFenceFree(void *p);
void HeapCheckPreFenceFree(void *p);

#ifdef __cplusplus
}
/*void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *p);
void operator delete[](void *p);*/

#endif /* __cplusplus */

#endif /* __PROTOTYP_H__ */
