// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014-2015 University of California
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

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include <vector>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include <unistd.h>
#endif

#include "browserlog.h"
#include "mongoose.h"
#include "webstatic.h"


static const char* index_html =
    "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'>"
    "<html>"
    "<head>"
    "    <title></title>"
    "    <script type='text/javascript' src='boinc.js'></script>"
    "</head>"
    "<body style='background-color: Black; overflow: hidden'>"
    "    <div id='content' style='position: absolute'>"
    "        <table>"
    "            <tr>"
    "                <td style='vertical-align: middle'>"
    "                    <img alt='' src='boinc.png' />"
    "                </td>"
    "                <td style='vertical-align: top; width: 250px'>"
    "                    <div id='username_container' style='visibility: hidden; color: White'></div>"
    "                    <div id='teamname_container' style='visibility: hidden; color: White'></div>"
    "                    <div id='workunitname_container' style='visibility: hidden; color: White'></div>"
    "                    <div id='progressbar_container' style='visibility: hidden; border: 1px Solid White; width: 245px; height: 23px'>"
    "                        <div id='progressbar_text' style='z-index: 1; position: absolute; color: White; text-align: center; vertical-align: middle; height: 21px; width: 245px'></div>"
    "                        <div id='progressbar' style='z-index: 0; position: absolute; height: 21px; width: 0px; background-color: Blue;'></div>"
    "                    </div>"
    "                    <div id='message_container' style='text-align: center; color: White'>Initializing...</div>"
    "                </td>"
    "            </tr>"
    "        </table>"
    "    </div>"
    "    <script type='text/javascript'>"
    "        var interval;"
    "        var step = 5;"
    "        var delay = 1000;"
    "        var height = 0;"
    "        var Hoffset = 0;"
    "        var Woffset = 0;"
    "        var yon = 0;"
    "        var xon = 0;"
    "        var xPos = 0;"
    "        var yPos = 0;"
    "        var boinc = new BOINC();"
    ""
    "        function refresh() {"
    "            boinc.poll();"
    ""
    "            var message = '';"
    ""
    "            if (boinc.getUserName().length != 0) {"
    "                document.getElementById('username_container').innerHTML = boinc.getUserName();"
    "            }"
    "            if (boinc.getTeamName().length != 0) {"
    "                document.getElementById('teamname_container').innerHTML = boinc.getTeamName();"
    "            }"
    "            if (boinc.getWorkunitName().length != 0) {"
    "                document.getElementById('workunitname_container').innerHTML = boinc.getWorkunitName();"
    "            }"
    ""
    "            document.getElementById('progressbar').style.width = "
    "                String(Math.floor(parseInt(document.getElementById('progressbar_container').style.width) * boinc.getFractionDone()) + 'px');"
    "            document.getElementById('progressbar_text').innerHTML = "
    "                String(Math.floor(100 * boinc.getFractionDone()) + ' %');"
    ""
    "            if (boinc.isSuspended()) {"
    "                message = 'Task is suspended.';"
    "            } else if (boinc.isNetworkSuspended()) {"
    "                message = 'Network is suspended.';"
    "            } else if (boinc.isExiting()) {"
    "                message = 'Exiting in ' + boinc.getExitTimeout() + ' seconds';"
    "            }"
    ""
    "            if ((document.getElementById('username_container').innerHTML.length == 0) || boinc.isExiting()) {"
    "                document.getElementById('username_container').style.visibility = 'hidden';"
    "                document.getElementById('teamname_container').style.visibility = 'hidden';"
    "                document.getElementById('workunitname_container').style.visibility = 'hidden';"
    "                document.getElementById('progressbar_container').style.visibility = 'hidden';"
    "                document.getElementById('message_container').style.visibility = 'visible';"
    "            } else {"
    "                document.getElementById('username_container').style.visibility = 'visible';"
    "                document.getElementById('teamname_container').style.visibility = 'visible';"
    "                document.getElementById('workunitname_container').style.visibility = 'visible';"
    "                document.getElementById('progressbar_container').style.visibility = 'visible';"
    "                document.getElementById('message_container').style.visibility = 'hidden';"
    "                if (message.length > 0) {"
    "                    document.getElementById('message_container').innerHTML = message;"
    "                    document.getElementById('message_container').style.textAlign = 'left';"
    "                    document.getElementById('message_container').style.visibility = 'visible';"
    "                } else {"
    "                    document.getElementById('message_container').innerHTML = '';"
    "                    document.getElementById('message_container').style.visibility = 'hidden';"
    "                }"
    "            }"
    ""
    "            update_positions();"
    "        }"
    ""
    "        function update_positions() {"
    "            if (navigator.appName == 'Microsoft Internet Explorer') {"
    "                width = document.body.clientWidth;"
    "                height = document.body.clientHeight;"
    "                document.getElementById('content').style.left = xPos + document.body.scrollLeft;"
    "                document.getElementById('content').style.top = yPos + document.body.scrollTop;"
    "            } else {"
    "                height = window.innerHeight;"
    "                width = window.innerWidth;"
    "                document.getElementById('content').style.top = yPos + window.pageYOffset;"
    "                document.getElementById('content').style.left = xPos + window.pageXOffset;"
    "            }"
    "            Hoffset = document.getElementById('content').offsetHeight;"
    "            Woffset = document.getElementById('content').offsetWidth;"
    ""
    "            if (yon) {"
    "                yPos = yPos + step;"
    "            } else {"
    "                yPos = yPos - step;"
    "            }"
    "            if (yPos < 0) {"
    "                yon = 1;"
    "                yPos = 0;"
    "            }"
    "            if (yPos >= (height - Hoffset)) {"
    "                yon = 0;"
    "                yPos = (height - Hoffset);"
    "            }"
    "            if (xon) {"
    "                xPos = xPos + step;"
    "            } else {"
    "                xPos = xPos - step;"
    "            }"
    "            if (xPos < 0) {"
    "                xon = 1;"
    "                xPos = 0;"
    "            }"
    "            if (xPos >= (width - Woffset)) {"
    "                xon = 0;"
    "                xPos = (width - Woffset);"
    "            }"
    "        }"
    ""
    "        interval = setInterval('refresh()', delay);"
    "    </script>"
    "</body>"
    "</html>";


int handle_static_index_html(struct mg_connection *conn) {
    mg_send_status(conn, 200);
    mg_send_header(conn, "Content-Type", "text/html");
    mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, pre-check=0, no-store, no-cache, must-revalidate");
    mg_send_header(conn, "Access-Control-Allow-Origin", "*");
    mg_printf_data(
        conn,
        "%s",
        index_html
    );
    return MG_TRUE;
}
