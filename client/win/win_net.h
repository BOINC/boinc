// $Id$
///////////////////////////////////////////////////////////////////////////////
// 
//				 Workfile:  Win_net.h
//                Net module
//				  Project:	SetiClient
//				  Created:	10/2/98 by Kyle Granger
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __WIN_NET_H
#define __WIN_NET_H

extern int NetOpen( void );
extern void NetClose( void );
extern void NetCheck( void );


#endif
/*
 * $Log$
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
