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

#ifndef h_BASE64
#define h_BASE64

#ifndef _WIN32
#include <cstdio>
#include <cstdlib>
#include <string>
using namespace std;
#endif

class InvalidBase64Exception
{
};

string r_base64_encode (const char* from, size_t length);
string r_base64_decode (const char* from, size_t length);
inline string r_base64_decode (string const& from)
{
    return r_base64_decode(from.c_str(), from.length());
}


#endif
