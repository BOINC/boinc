// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,

#ifndef __WIN_NET_H
#define __WIN_NET_H

extern int WinsockInitialize();
extern int WinsockCleanup();

extern int  NetOpen( void );
extern void NetClose( void );
extern void NetCheck( void );

#endif
/*
 * $Log$
 * Revision 1.5  2005/01/20 23:20:22  boincadm
 * *** empty log message ***
 *
 * Revision 1.4  2004/06/09 18:17:24  rwalton
 * *** empty log message ***
 *
 * Revision 1.3  2004/03/04 11:41:41  rwalton
 * *** empty log message ***
 *
 * Revision 1.2  2002/12/18 20:20:04  eheien
 * confirm before connecting
 *
 * Revision 1.1  2002/08/09 21:43:19  eheien
 * Moved win files, fixed compile bugs, added user requestable quit.
 *
 * Revision 1.1  2002/06/21 00:13:48  eheien
 * Windows networking support.
 *
 * Revision 4.0  2000/10/05 18:04:58  korpela
 * Synchronized versions to 4.0 following release of 3.0 client
 *
 */
