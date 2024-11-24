// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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
#include "filesys.h"
#include "parse.h"
#include "str_replace.h"
#include "util.h"

#include "str_util.h"
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
    optional = false;
    no_validate = false;
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/file_ref")) {
            if (phys_name.empty()) return ERR_XML_PARSE;
            if (logical_name.empty()) return ERR_XML_PARSE;
            return 0;
        }
        if (xp.parse_string("file_name", phys_name)) {
            continue;
        }
        if (xp.parse_string("open_name", logical_name)) {
            continue;
        }
        if (xp.parse_bool("optional", optional)) continue;
        if (xp.parse_bool("no_validate", no_validate)) continue;
    }
    return ERR_XML_PARSE;
}

int get_output_file_info(RESULT const& result, OUTPUT_FILE_INFO& fi) {
    char path[MAXPATHLEN];
    string name;
    MIOFILE mf;
    mf.init_buf_read(result.xml_doc_in);
    XML_PARSER xp(&mf);
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("file_ref")) {
            int retval = fi.parse(xp);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "get_output_file_info(): error parsing %s\n",
                    result.xml_doc_in
                );
                return retval;
            }
            if (standalone) {
                safe_strcpy(path, fi.phys_name.c_str());
                if (!path_to_filename(fi.phys_name, name)) {
                    fi.phys_name = name;
                }
            } else {
                dir_hier_path(
                    fi.phys_name.c_str(), config.upload_dir,
                    config.uldl_dir_fanout, path
                );
            }
            fi.path = path;
            return 0;
        }
    }
    return ERR_XML_PARSE;
}

int get_output_file_infos(RESULT const& result, vector<OUTPUT_FILE_INFO>& fis) {
    char path[MAXPATHLEN];
    string name;
    MIOFILE mf;
    mf.init_buf_read(result.xml_doc_in);
    XML_PARSER xp(&mf);
    fis.clear();
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("file_ref")) {
            OUTPUT_FILE_INFO fi;
            int retval =  fi.parse(xp);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "get_output_file_infos(): error parsing %s\n",
                    result.xml_doc_in
                );
                return retval;
            }
            if (standalone) {
                safe_strcpy(path, fi.phys_name.c_str());
                if (!path_to_filename(fi.phys_name, name)) {
                    fi.phys_name = name;
                }
            } else {
                dir_hier_path(
                    fi.phys_name.c_str(), config.upload_dir,
                    config.uldl_dir_fanout, path
                );
            }
			fi.path = path;
            fis.push_back(fi);
        }
    }
    return 0;
}

int get_output_file_path(RESULT const& result, string& path) {
    OUTPUT_FILE_INFO fi;
    int retval = get_output_file_info(result, fi);
    if (retval) return retval;
    path = fi.path;
    return 0;
}

int get_output_file_paths(RESULT const& result, vector<string>& paths) {
    vector<OUTPUT_FILE_INFO> fis;
    int retval = get_output_file_infos(result, fis);
    if (retval) return retval;
    paths.clear();
    for (unsigned int i=0; i<fis.size(); i++) {
        paths.push_back(fis[i].path);
    }
    return 0;
}

// remove the random part of an output filename:
// given a name of the form "xxx_resultnum_r123123_filenum",
// return "xxx_resultnum_filenum"
//
void remove_random_from_filename(const char* in, char* out) {
    strcpy(out, in);
    const char* p_in = strrchr(in, 'r');
    if (!p_in) return;
    if (p_in == in) return;
    if (*(--p_in) != '_') return;
    char* p_out = out + (p_in - in);
    const char *q = strchr(p_in+1, '_');
    if (q) {
        strcpy(p_out, q);
    } else {
        strcpy(p_out, "");
    }
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
    char buf[1024], phys_name[1024];
    MIOFILE mf;
    int retval;

    mf.init_buf_read(result.xml_doc_in);
    XML_PARSER xp(&mf);

    safe_strcpy(buf, path.c_str());
    char* p = strrchr(buf, '/');
    if (!p) return ERR_NOT_FOUND;
    safe_strcpy(phys_name, p+1);

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
