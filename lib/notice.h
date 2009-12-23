// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2009 University of California
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

#ifndef __NOTICE_H__
#define __NOTICE_H__

#include <string>

#include "parse.h"

// represents a notice delivered from client to GUI

struct NOTICE {
    int seqno;
    char title[256];
    std::string description;
    double create_time;
    double arrival_time;     // when item arrived at client
    bool is_private;
    char category[64];
    char url[256];

    // the following fields used in client only
    char guid[256];

    int parse(XML_PARSER&);
    int parse_rss(XML_PARSER&);
    void write(MIOFILE&, bool for_gui);
};

#endif
