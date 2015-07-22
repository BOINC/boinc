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

#ifndef _WEBAPI_H_
#define _WEBAPI_H_

int    determine_exit_state(double& exit_timeout);
int    determine_state_url(std::string& url);

void   handle_poll_server();

int    handle_get_init_data(struct mg_connection *conn);
int    handle_get_graphics_status(struct mg_connection *conn);
int    handle_reset_read_flag(struct mg_connection *conn);
int    handle_filesystem_request(struct mg_connection *conn);

#endif
