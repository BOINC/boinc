// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#ifndef BOINC_DAEMONMGT_H
#define BOINC_DAEMONMGT_H

extern bool is_daemon_installed();
extern bool is_daemon_starting();
extern bool is_daemon_running();
extern bool is_daemon_stopping();
extern bool is_daemon_stopped();

extern bool start_daemon_via_daemonctrl();
extern bool start_daemon();
extern bool stop_daemon_via_daemonctrl();
extern bool stop_daemon();

#endif
