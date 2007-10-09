// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef h_MD5_FILE
#define h_MD5_FILE

#include <string>

// length of buffer to hold an MD5 hash
#define MD5_LEN 64

extern int md5_file(const char* path, char* output, double& nbytes);
extern int md5_block(const unsigned char* data, int nbytes, char* output);

extern std::string md5_string(const unsigned char* data, int nbytes);

inline std::string md5_string(std::string const& data)
{
    return md5_string((const unsigned char*) data.c_str(), (int)data.size());
}

extern int make_random_string(char*);
#endif
