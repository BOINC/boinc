// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifndef h_MD5_FILE
#define h_MD5_FILE

#include <string>

// length of buffer to hold an MD5 hash
#define MD5_LEN 64

extern int md5_file(const char* path, char* output, double& nbytes);
extern int md5_block(const unsigned char* data, int nbytes, char* output);

std::string md5_string(const unsigned char* data, int nbytes);

inline std::string md5_string(std::string const& data)
{
    return md5_string((const unsigned char*) data.c_str(), data.size());
}

#endif
