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

#define URL_PROTOCOL_UNKNOWN 0
#define URL_PROTOCOL_HTTP    1
#define URL_PROTOCOL_HTTPS   2
#define URL_PROTOCOL_SOCKS   3

extern void parse_url(
    const char* url, int &protocol, char* host, int &port, char* file
);
extern void unescape_url(std::string& url);
extern void unescape_url(char *url);
extern void escape_url(std::string& url);
extern void escape_url(char *in, char*out);
extern void escape_url_readable(char* in, char* out);
extern void escape_project_url(char *in, char* out);
extern bool valid_master_url(char*);
extern void canonicalize_master_url(char *url);
extern void canonicalize_master_url(std::string&);
