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

#include <stdbool.h>
#include "miofile.h"
#include "error_numbers.h"
#include "certificate.h"

CERTIFICATE::CERTIFICATE() {
    this->clear();
}

CERTIFICATE::~CERTIFICATE() {
    // TODO
}

void CERTIFICATE::clear() {
    this->type = MD5_HASH;     // md5 hash by default
    memset(this->subject, 0, sizeof(this->subject));
    memset(this->signature, 0, sizeof(this->signature));    
}

CERTIFICATES::CERTIFICATES() {
    // TODO
}

CERTIFICATES::~CERTIFICATES() {
    // TODO
}

void CERTIFICATES::clear() {
    this->signatures.clear();
}

int CERTIFICATES::count() {
    return this->signatures.size();
}

int CERTIFICATES::parse(XML_PARSER &xp) {
    CERTIFICATE sig;
    int is_tag = false;
    int in_entry = false;
    int in_sig = false;
    int parsed_one = false;
    char tag[4096];
    char buf[256];
    
    //printf("CERTIFICATES::parse() starts.\n");
    //fflush(stdout);
    
    while (!xp.get(tag, sizeof(tag), (bool&)is_tag)) {
        if (!strcmp(tag, "/signatures")) {
            //printf("CERTIFICATES::parse() ends.\n");
            //fflush(stdout);
            return !in_entry && !in_sig && parsed_one;
        }
        if (in_sig) {
            in_sig = false;
            snprintf(sig.signature, sizeof(sig.signature), "%s", tag);
            continue;
        } 
        if (!is_tag) {
            printf("(CERTIFICATES): unexpected text: %s\n", tag);
            continue;
        }
        if (in_entry) {
            if (!strcmp(tag, "/entry")) {
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
            if (!strcmp(tag, "signature")) {
                in_sig = true;
                continue;
            }
            if (!strcmp(tag, "/signature")) {
                in_sig = false;
                continue;
            }
            if (xp.parse_str(tag, "subject", sig.subject, sizeof(sig.subject)))
                continue;
            else if (xp.parse_str(tag, "hash", sig.hash, sizeof(sig.hash)))
                continue;
            else if (xp.parse_str(tag, "type", buf, sizeof(buf))) {
                if ((!strcmp(buf,"md5")) || (!strcmp(buf,"MD5"))) {
                    sig.type = MD5_HASH;
                } else if ((!strcmp(buf,"sha1")) || (!strcmp(buf,"SHA1"))) {
                    sig.type = SHA1_HASH;                    
                }
                continue;
            }
        } else {
            if (strstr(tag, "entry")) {
                in_entry = true;
                continue;
            }
        }
    
    }
    return false;
}

int CERTIFICATES::parse_miofile_embed(MIOFILE &mf) {
    XML_PARSER xp(&mf);
    return this->parse(xp);    
}

int CERTIFICATES::parse_file(const char* filename) {
    FILE* f;
    int retval;

    f = fopen(filename, "r");
    if (!f) 
        return ERR_FOPEN;
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

int CERTIFICATES::parse_buffer(char* buf) {
    MIOFILE mf;
    int retval;

    mf.init_buf_read(buf);
    XML_PARSER xp(&mf);
    retval = this->parse(xp);
    return retval;
}

int CERTIFICATES::parse_buffer_embed(char* buf) {
    MIOFILE mf;
    char tag[4096];
    int is_tag;
    int s_found = false;

    mf.init_buf_read(buf);
    XML_PARSER xp(&mf);
    while (!xp.get(tag, sizeof(tag), (bool&)is_tag)) {
        if (!strcmp(tag, "signatures")) {
            s_found = true;
            break;
        }
    }
    if (s_found)
        return this->parse(xp);
    else
        return false;
}

void CERTIFICATES::dump() {
    MIOFILE m;
    char buf[4096];
    
    m.init_buf_write((char *)buf, 4096);
    this->write(m, 4096);
    printf("%s", buf);
}

int CERTIFICATES::write(MIOFILE &f) {
    if (this->signatures.size()==0) 
        return true;
    f.printf("<signatures>\n");
    for(int i=0;i < this->signatures.size(); i++) {
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

