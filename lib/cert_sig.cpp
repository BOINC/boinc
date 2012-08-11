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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#include "config.h"
#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#else
#include <cstdio>
#endif
#endif

#include "miofile.h"
#include "error_numbers.h"

#include "cert_sig.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

CERT_SIG::CERT_SIG() {
    this->clear();
}

CERT_SIG::~CERT_SIG() {
    // TODO
}

void CERT_SIG::clear() {
    this->type = MD5_HASH;     // md5 hash by default
    memset(this->subject, 0, sizeof(this->subject));
    memset(this->signature, 0, sizeof(this->signature));    
}

CERT_SIGS::CERT_SIGS() {
    // TODO
}

CERT_SIGS::~CERT_SIGS() {
    // TODO
}

void CERT_SIGS::clear() {
    this->signatures.clear();
}

int CERT_SIGS::count() {
    return (int)(this->signatures.size());
}

int CERT_SIGS::parse(XML_PARSER &xp) {
    CERT_SIG sig;
    bool in_entry = false;
    bool in_sig = false;
    bool parsed_one = false;
    char buf[256];
    
    while (!xp.get_tag()) {
        if (xp.match_tag("/signatures")) {
            //printf("CERT_SIGS::parse() ends.\n");
            //fflush(stdout);
            return !in_entry && !in_sig && parsed_one;
        }
        if (in_sig) {
            in_sig = false;
            snprintf(sig.signature, sizeof(sig.signature), "%s", xp.parsed_tag);
            continue;
        } 
        if (!xp.is_tag) {
            printf("(CERT_SIGS): unexpected text: %s\n", xp.parsed_tag);
            continue;
        }
        if (in_entry) {
            if (xp.match_tag("/entry")) {
                in_entry = false;
                in_sig = false;
                if (strlen(sig.subject) == 0) {
                    printf("ERROR: subject is not set.\n");
                    return false;
                }
                if (strlen(sig.signature) == 0) {
                    printf("ERROR: signature is not set.\n");
                    return false;
                }
                this->signatures.push_back(sig);
                parsed_one = true;
                sig.clear();                
                continue;
            }
            if (xp.match_tag("signature")) {
                in_sig = true;
                continue;
            }
            if (xp.match_tag("/signature")) {
                in_sig = false;
                continue;
            }
            if (xp.parse_str("subject", sig.subject, sizeof(sig.subject)))
                continue;
            else if (xp.parse_str("hash", sig.hash, sizeof(sig.hash)))
                continue;
            else if (xp.parse_str("type", buf, sizeof(buf))) {
                if ((!strcmp(buf,"md5")) || (!strcmp(buf,"MD5"))) {
                    sig.type = MD5_HASH;
                } else if ((!strcmp(buf,"sha1")) || (!strcmp(buf,"SHA1"))) {
                    sig.type = SHA1_HASH;                    
                }
                continue;
            }
        } else {
            if (strstr(xp.parsed_tag, "entry")) {
                in_entry = true;
                continue;
            }
        }
    
    }
    return false;
}

int CERT_SIGS::parse_file(const char* filename) {
    int retval;

#ifndef _USING_FCGI_
    FILE *f = fopen(filename, "r");
#else
    FCGI_FILE *f = FCGI::fopen(filename, "r");
#endif
    if (!f) return ERR_FOPEN;
    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);
    if (!xp.parse_start("signatures")) {
        return ERR_XML_PARSE;
    }
    retval = this->parse(xp);
    fclose(f);
    return retval;
}

int CERT_SIGS::parse_buffer(char* buf) {
    MIOFILE mf;
    int retval;

    mf.init_buf_read(buf);
    XML_PARSER xp(&mf);
    retval = this->parse(xp);
    return retval;
}

int CERT_SIGS::parse_buffer_embed(char* buf) {
    MIOFILE mf;
    char tag[4096];
    bool is_tag;
    int s_found = false;

    mf.init_buf_read(buf);
    XML_PARSER xp(&mf);
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (xp.match_tag("signatures")) {
            s_found = true;
            break;
        }
    }
    if (s_found)
        return this->parse(xp);
    else
        return false;
}

int CERT_SIGS::write(MIOFILE &f) {
    if (this->signatures.size()==0) return true;
    f.printf("<signatures>\n");
    for(unsigned int i=0;i < this->signatures.size(); i++) {
        f.printf("  <entry>\n");
        f.printf("    <signature>\n%s\n", this->signatures.at(i).signature);
        f.printf("    </signature>\n");
        f.printf("    <subject>%s</subject>\n", this->signatures.at(i).subject);    
        f.printf("    <type>%s</type>\n", (this->signatures.at(i).type == MD5_HASH) ? "md5" : "sha1");
        f.printf("    <hash>%s</hash>\n", this->signatures.at(i).hash);
        f.printf("  </entry>\n");
    }
    f.printf("</signatures>\n");
    return true;
}

