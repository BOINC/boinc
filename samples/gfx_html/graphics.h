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

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

int parse_graphics(
    std::string& default_url,
    std::string& running_url,
    std::string& suspended_url,
    std::string& network_suspended_url,
    std::string& exiting_url
);

#endif
