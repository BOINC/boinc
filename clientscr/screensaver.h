// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef __SCREENSAVER_H__
#define __SCREENSAVER_H__

// Default values, overriden by screensaver config file ss_config.xml
#define GFX_BLANK_PERIOD 0     /* 0 minutes */
#define GFX_DEFAULT_PERIOD 120 /* if default OpenGL screensaver exists, display it for 2 minutes */
#define GFX_SCIENCE_PERIOD 600 /* Display various science graphics apps for 10 minutes */
#define GFX_CHANGE_PERIOD 300  /* if > 1 CPUs, change screensaver every 5 minutes */

enum SS_PHASE {
    DEFAULT_SS_PHASE,
    SCIENCE_SS_PHASE
};

//-----------------------------------------------------------------------------
// Error / status codes
//-----------------------------------------------------------------------------

// These codes are no longer used by the Mac:
#define SCRAPPERR_BOINCNOTDETECTED                          0x82000001
#define SCRAPPERR_BOINCNOTDETECTEDSTARTUP                   0x82000002
#define SCRAPPERR_BOINCSUSPENDED                            0x82000003
#define SCRAPPERR_BOINCNOTGRAPHICSCAPABLE                   0x82000004
#define SCRAPPERR_BOINCNOAPPSEXECUTING                      0x82000005
#define SCRAPPERR_BOINCNOPROJECTSDETECTED                   0x82000006
#define SCRAPPERR_BOINCNOGRAPHICSAPPSEXECUTING              0x82000007
#define SCRAPPERR_BOINCAPPFOUNDGRAPHICSLOADING              0x82000009
#define SCRAPPERR_BOINCSHUTDOWNEVENT                        0x8200000a
#define SCRAPPERR_NOPREVIEW                                 0x8200000f
#define SCRAPPERR_DAEMONALLOWSNOGRAPHICS                    0x82000010
#define SCRAPPERR_SCREENSAVERRUNNING                        0x82000011
#define SCRAPPERR_QUITSCREENSAVERREQUESTED                  0x82000013

// The following are still used by the Mac:
#define SCRAPPERR_BOINCSCREENSAVERLOADING                   0x82000008
#define SCRAPPERR_SCREENSAVERBLANKED                        0x82000012

// The following are new codes used by the Mac:
#define SCRAPPERR_CANTLAUNCHDEFAULTGFXAPP                   0x82000014
#define SCRAPPERR_DEFAULTGFXAPPCANTCONNECT                  0x82000015
#define SCRAPPERR_DEFAULTGFXAPPCRASHED                      0x82000016
#define SCRAPPERR_GFXAPPINCOMPATIBLE                        0x82000017


#endif
