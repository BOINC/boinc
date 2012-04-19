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

// Support functions for validators:
// 1) functions for locating the output files
// 2) various ways of deciding how much credit to grant
//    a group of replicated results

#include <cstring>
#include "config.h"

#include "error_numbers.h"
#include "parse.h"
#include "util.h"
#include "filesys.h"

#include "sched_util.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "validator.h"
#include "validate_util.h"

using std::vector;
using std::string;

bool standalone = false;

////////// functions for locating output files ///////////////

int OUTPUT_FILE_INFO::parse(XML_PARSER& xp) {
    bool found=false;
    optional = false;
    no_validate = false;
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/file_ref")) {
            return found?0:ERR_XML_PARSE;
        }
        if (xp.parse_string("file_name", name)) {
            found = true;
            continue;
        }
        if (xp.parse_bool("optional", optional)) continue;
        if (xp.parse_bool("no_validate", no_validate)) continue;
    }
    return ERR_XML_PARSE;
}

int get_output_file_info(RESULT& result, OUTPUT_FILE_INFO& fi) {
    char path[1024];
    string name;
    MIOFILE mf;
    mf.init_buf_read(result.xml_doc_in);
    XML_PARSER xp(&mf);
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("file_ref")) {
            int retval = fi.parse(xp);
            if (retval) return retval;
            if (standalone) {
                strcpy(path, fi.name.c_str());
            } else {
                dir_hier_path(
                    fi.name.c_str(), config.upload_dir,
                    config.uldl_dir_fanout, path
                );
            }
            fi.path = path;
            return 0;
        }
    }
    return ERR_XML_PARSE;
}

int get_output_file_infos(RESULT& result, vector<OUTPUT_FILE_INFO>& fis) {
    char path[1024];
    MIOFILE mf;
    string name;
    mf.init_buf_read(result.xml_doc_in);
    XML_PARSER xp(&mf);
    fis.clear();
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("file_ref")) {
            OUTPUT_FILE_INFO fi;
            int retval =  fi.parse(xp);
            if (retval) return retval;
            if (standalone) {
                strcpy(path, fi.name.c_str());
            } else {
                dir_hier_path(
                    fi.name.c_str(), config.upload_dir,
                    config.uldl_dir_fanout, path
                );
            }
            fi.path = path;
            fis.push_back(fi);
        }
    }
    return 0;
}

int get_output_file_path(RESULT& result, string& path) {
    OUTPUT_FILE_INFO fi;
    int retval = get_output_file_info(result, fi);
    if (retval) return retval;
    path = fi.path;
    return 0;
}

int get_output_file_paths(RESULT& result, vector<string>& paths) {
    vector<OUTPUT_FILE_INFO> fis;
    int retval = get_output_file_infos(result, fis);
    if (retval) return retval;
    paths.clear();
    for (unsigned int i=0; i<fis.size(); i++) {
        paths.push_back(fis[i].path);
    }
    return 0;
}

struct FILE_REF {
    char file_name[256];
    char open_name[256];
    int parse(XML_PARSER& xp) {
        strcpy(file_name, "");
        strcpy(open_name, "");
        while (!xp.get_tag()) {
            if (!xp.is_tag) continue;
            if (xp.match_tag("/file_ref")) {
                return 0;
            }
            if (xp.parse_str("file_name", file_name, sizeof(file_name))) continue;
            if (xp.parse_str("open_name", open_name, sizeof(open_name))) continue;
        }
        return ERR_XML_PARSE;
    }
};

// given a path returned by the above, get the corresponding logical name
//
int get_logical_name(RESULT& result, string& path, string& name) {
    char phys_name[1024];
    MIOFILE mf;
    int retval;

    mf.init_buf_read(result.xml_doc_in);
    XML_PARSER xp(&mf);

    strcpy(phys_name, path.c_str());
    char* p = strrchr(phys_name, '/');
    if (!p) return ERR_NOT_FOUND;
    strcpy(phys_name, p+1);

    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("result")) continue;
        if (xp.match_tag("file_ref")) {
            FILE_REF fr;
            retval = fr.parse(xp);
            if (retval) continue;
            if (!strcmp(phys_name, fr.file_name)) {
                name = fr.open_name;
                return 0;
            }
            continue;
        }
        xp.skip_unexpected(false, 0);
    }
    return ERR_XML_PARSE;
}

int get_credit_from_wu(WORKUNIT& wu, vector<RESULT>&, double& credit) {
    double x;
    int retval;
    DB_WORKUNIT dbwu;
    
    dbwu.id = wu.id;
    retval = dbwu.get_field_str("xml_doc", dbwu.xml_doc, sizeof(dbwu.xml_doc));
    if (!retval) {
        if (parse_double(dbwu.xml_doc, "<credit>", x)) {
            credit = x;
            return 0;
        }
    }
    return ERR_XML_PARSE;
}

const char *BOINC_RCSID_07049e8a0e = "$Id$";
