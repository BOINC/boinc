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

#ifndef __SIGNATURES_H_
#define __SIGNATURES_H_

#include <vector>
#include "parse.h"

#ifndef MAX_SIGNATURE_LEN
#define MAX_SIGNATURE_LEN 4096
#endif
#define MAX_SUBJECT_LEN 256
#define MD5_HASH     0
#define SHA1_HASH    1

struct SIGNATURE {
public:
    char signature[MAX_SIGNATURE_LEN];  // RSA signature expected.
    int type;                           // MD5_HASH or SHA1_HASH. Not used yet.
    char subject[MAX_SUBJECT_LEN];
    char hash[9];                       // 8 + '\0'...
    SIGNATURE();
    ~SIGNATURE();
    void clear();
};

class SIGNATURES {
public:
    std::vector<SIGNATURE> signatures;
    SIGNATURES();
    ~SIGNATURES();
    //
    // Parses an .xml signature file with the following structure:
    //
    //<signatures>
    //	<entry>
    //		<thesignature>
    //%signature%
    //		</thesignature>
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

#endif  //__SIGNATURES_H_
