// $Id$
//
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
// Revision History:
//
// $Log$
// Revision 1.7  2005/01/02 18:29:21  ballen
// Modified CVS id strings.  After some fussing with different versions
// of gcc to try and force them to not complain with -Wall but to always
// include this, I decided to take a simpler approach.  All these strings
// now have global linkage.  To prevent namespace conflicts they all
// have different names.  For the record, the variable extension is a hash made of the first ten characters of the md5sum of the file path, eg:
//     md5hash=`boinc/api/x_opengl.C | md5sum | cut -c 1-10`
//
// Revision 1.6  2004/12/08 00:39:13  ballen
// Moved RCSID strings to the end of all .c, .C and .cpp files as per
// David's request.
//
// Revision 1.5  2004/12/02 20:17:34  rwalton
// *** empty log message ***
//
// Revision 1.4  2004/05/17 22:15:10  rwalton
// *** empty log message ***
//
//

#include "stdwx.h"

#ifdef __WXMSW__

#ifndef __WIN95__
#error An error in wxWindows version 2.4.0 and 2.4.1 require that you set WINVER=0x400 in the project settings!
#endif

#endif

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


const char *BOINC_RCSID_4e0e4c54ab = "$Id$";
