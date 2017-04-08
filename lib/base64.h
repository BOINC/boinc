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

#ifndef BOINC_BASE64_H
#define BOINC_BASE64_H

#ifndef _WIN32
#include <cstdio>
#include <cstdlib>
#include <string>
#endif

class InvalidBase64Exception
{
};


std::string r_base64_encode(const char* from, size_t length);
std::string r_base64_decode(const char* from, size_t length);


inline std::string r_base64_encode(std::string const& from)
{
    return r_base64_encode(from.c_str(), from.length());
}
inline std::string r_base64_decode(std::string const& from)
{
    return r_base64_decode(from.c_str(), from.length());
}

#endif
