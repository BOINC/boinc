// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#ifndef __WIN_NET_H
#define __WIN_NET_H

extern int NetOpen( void );
extern void NetClose( void );
extern void NetCheck( void );


#endif
/*
 * $Log$
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
