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

typedef enum ENetState {
	NETSTATE_NONE = 0,
	NETSTATE_WAIT,
	NETSTATE_NOTIFY,
	NETSTATE_PERMISSION,
	NETSTATE_LAST
} ENetState;

extern int NetOpen( void );
extern void NetClose( void );
extern void NetSetState( ENetState state );


#endif
/*
 * $Log$
 * Revision 1.1  2002/06/21 00:13:48  eheien
 * Windows networking support.
 *
 * Revision 4.0  2000/10/05 18:04:58  korpela
 * Synchronized versions to 4.0 following release of 3.0 client
 *
 */
