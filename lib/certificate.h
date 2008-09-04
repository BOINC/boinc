//
// Copyright (C) 2006-2008 MTA SZTAKI
//
// Marosi Attila Csaba <atisu@sztaki.hu>
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

#ifndef __CERT_SIG_H_
#define __CERT_SIG_H_

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

class CERT_SIGS {
public:
    std::vector<CERT_SIG> cert_sigs;
    CERT_SIG();
    ~CERT_SIG();
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
    int write(MIOFILE &f, int max);
    // Parses from an already opened MIOFILE, the pointer should have
    // passed the opening <signatures> tag (no check is done for that).
    int parse_miofile_embed(MIOFILE &mf);
    int parse_buffer_embed(char* buf);
    void dump();
    void clear();
    int count();        // return the total number of signatures.
private:
    int parse(XML_PARSER &xp);
};

#endif  //__CERT_SIG_H_
