// $Id$
///////////////////////////////////////////////////////////////////////////////
// 
//				 Workfile:  Win_net.cpp
//                Net module
//				  Project:	SetiClient
//				  Created:	12/20/98 by Kyle Granger
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "win_net.h"
#include "win_util.h"


//////////////////////////////////////////////////////////////////////////////////////
//
// Function    : NetOpen
//
//
///////////////////////////////////////////////////////////////////////////////////////


int NetOpen( void )
{
    WSADATA wsdata;
    WORD    wVersionRequested;
	int rc, addrlen = 16;

	// return if already open
	//if ( Globals->status & STATUS_FLAG_NETOPEN ) return 0;


	// Handle permission logic here
	//
	/*if ( config.ConnectionType == CONNECTION_NOTIFYME && Globals->net_state != NETSTATE_PERMISSION )
	{
		if ( !(Globals->status & STATUS_FLAG_SAVER) && (Globals->status & STATUS_FLAG_MAXIMIZED) )
		{
			// If the screen saver is not running and we are maximized, go ahead 
			// and ask permission to log on
			ConnectionDlg();
			if ( !connection.ConnectNow )
			{
				return NET_WAIT;
			}
		}
		else
		{
			return NET_NOTIFYME;
		}
	}*/

    wVersionRequested = MAKEWORD(1,1);
    rc = WSAStartup(wVersionRequested, &wsdata);

    if (rc) 
	{
        //printf("WSAStartup failed: error code = %d\n", rc);
        return -1; //CANT_CONNECT;
    }


	//Globals->status |= STATUS_FLAG_NETOPEN;
	NetSetState( NETSTATE_NONE );

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////
//
// Function    : NetClose
//
//
///////////////////////////////////////////////////////////////////////////////////////

void NetClose( void )
{
	/*if ( !(Globals->status & STATUS_FLAG_NETOPEN) ) 
		return;*/

	/*Globals->status &= ~(STATUS_FLAG_NETOPEN|STATUS_FLAG_TRANSFER);*/
	//Globals->status &= ~STATUS_FLAG_NETOPEN;
 	WSACleanup();
}


//////////////////////////////////////////////////////////////////////////////////////
//
// Function    : NetSetState()
//
//
///////////////////////////////////////////////////////////////////////////////////////

void NetSetState( ENetState state )
{
	/*Globals->net_state = state;
	switch( state )
	{
	case NETSTATE_WAIT:
		if ( !(Globals->status & STATUS_FLAG_SAVER) )
		{
			EnableMenuItem( Globals->FileMenu, ID_FILE_CONNECTNOW, 
				MF_BYCOMMAND | MF_ENABLED );
			EnableMenuItem( Globals->PopupSubMenu, ID_POPUP_CONNECTNOW, 
				MF_BYCOMMAND | MF_ENABLED );
			UtilSetBlink( FALSE );
		}
		else
			NetSetState( NETSTATE_NOTIFY );
		break;

	case NETSTATE_NOTIFY:
		gdata->set_alert_info( TRUE );
		UtilSetBlink( TRUE );
		break;

	case NETSTATE_PERMISSION:
		gdata->set_alert_info( FALSE );
		UtilStartWorker();
		UtilSetBlink( FALSE );
		EnableMenuItem( Globals->FileMenu, ID_FILE_CONNECTNOW, 
			MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( Globals->PopupSubMenu, ID_POPUP_CONNECTNOW, 
			MF_BYCOMMAND | MF_GRAYED );
		break;

	case NETSTATE_NONE:
		break;
	}*/
}

/*
 * $Log$
 * Revision 1.1  2002/06/21 00:13:48  eheien
 * Windows networking support.
 *
 * Revision 4.0  2000/10/05 18:04:58  korpela
 * Synchronized versions to 4.0 following release of 3.0 client
 *
 */


