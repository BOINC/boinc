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

#ifndef h_MD5_FILE
#define h_MD5_FILE

#include <string>

// length of buffer to hold an MD5 hash
#define MD5_LEN 64

extern int md5_file(const char* path, char* output, double& nbytes);
extern int md5_block(const unsigned char* data, int nbytes, char* output);

extern std::string md5_string(const unsigned char* data, int nbytes);

inline std::string md5_string(std::string const& data) {
    return md5_string((const unsigned char*) data.c_str(), (int)data.size());
}

extern int make_random_string(char*);
#endif
