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

#ifndef BOINC_CERT_SIG_H
#define BOINC_CERT_SIG_H

#include <vector>
#include "parse.h"

#define MAX_CERT_SIG_LEN 4096
#define MAX_SUBJECT_LEN 256
#define MD5_HASH     0
#define SHA1_HASH    1

struct CERT_SIG {
    char signature[MAX_CERT_SIG_LEN];  // RSA signature expected.
    int type;                           // MD5_HASH or SHA1_HASH. Not used yet.
    char subject[MAX_SUBJECT_LEN];
    char hash[9];                       // 8 + '\0'...
    CERT_SIG();
    ~CERT_SIG();
    void clear();
};

struct CERT_SIGS {
    std::vector<CERT_SIG> signatures;
    CERT_SIGS();
    ~CERT_SIGS();
    //
    // Parses an .xml signature file with the following structure:
    //
    //<signatures>
    //	<entry>
    //		<signature>
    //%signature%
    //		</signature>
    //	    <subject>%certificate_subject%</subject>
    //      <type>%md5_or_sha1(_with_rsa)%</type>
    //      <hash>%certificate_hash%</hash>
    //	</entry>
    //	<entry>
    //	    ...
    //	</entry>
    //	...
    //</signatures>
    int parse_file(const char* filename);
    int parse_buffer(char* buf);
    int write(MIOFILE &f);
    int parse_buffer_embed(char* buf);
    void clear();
    int count();        // return the total number of signatures.
    int parse(XML_PARSER &xp);
};

#endif  // BOINC_CERT_SIG_H
