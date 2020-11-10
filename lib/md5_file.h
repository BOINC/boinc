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

#ifndef BOINC_MD5_FILE_H
#define BOINC_MD5_FILE_H

#include <string>

// length of buffer to hold an MD5 hash
// In principle need 32 + 1 for NULL,
// but leave some room for XML whitespace
// (since we parse before stripping whitespace)
//
#define MD5_LEN 64

extern int md5_file(
    const char* path, char* output, double& nbytes, bool is_gzip=false
);

extern int md5_block(const unsigned char* data, int nbytes, char* output,
    const unsigned char* data2=0, int nbytes2=0
);

extern std::string md5_string(const unsigned char* data, int nbytes);

inline std::string md5_string(std::string const& data) {
    return md5_string((const unsigned char*) data.c_str(), (int)data.size());
}

extern int make_secure_random_string_os(char*);
#endif
