
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

// parse BUDA app and variant description files


#include <nlohmann/json.hpp>
using nlohmann::json;

#include "filesys.h"

#include "sched_customize.h"
#include "sched_main.h"
#include "sched_msgs.h"

#include "buda.h"

#define BUDA_APPS_DIR "../buda_apps"

BUDA_APPS buda_apps;

// scan a dir that's possibly a BUDA variant
//
bool BUDA_VARIANT::read_json(string app_name, string var_name) {
    char path[MAXPATHLEN];
    sprintf(path, "%s/%s/%s/%s",
        BUDA_APPS_DIR,
        app_name.c_str(),
        var_name.c_str(),
        "variant.json"
    );
    if (!boinc_file_exists(path)) return false;
    string s;
    read_file_string(path, s);
    json d;
    try {
        d = json::parse(s);
    } catch (const std::exception& e) {
        log_messages.printf(MSG_CRITICAL,
            "BUDA variant: failed to parse %s: %s\n", path, e.what()
        );
        return false;
    }
    if (d.find("file_infos") == d.end()
        || d.find("file_refs") == d.end()
        || d.find("cpu_type") == d.end()
        || d.find("plan_class") == d.end()
    ) {
        log_messages.printf(MSG_CRITICAL,
            "BUDA variant: missing element in %s\n", path
        );
        return false;
    }
    file_infos = d["file_infos"].get<string>();
    file_refs = d["file_refs"].get<string>();
    string ct = d["cpu_type"].get<string>();
    if (ct == "intel") {
        cpu_type = CPU_TYPE_INTEL;
    } else if (ct == "arm") {
        cpu_type = CPU_TYPE_ARM;
    } else {
        cpu_type = CPU_TYPE_UNKNOWN;
    }
    plan_class = d["plan_class"].get<string>();
    return true;
}

// scan a dir that's possibly a BUDA app;
// return true if success
// false if not a BUDA app, or error
//
bool BUDA_APP::read_json() {
    char path[MAXPATHLEN];
    sprintf(path, "%s/%s/%s",
        BUDA_APPS_DIR,
        name.c_str(),
        "desc.json"
    );
    if (!boinc_file_exists(path)) {
        return false;
    }
    string s;
    read_file_string(path, s);
    json d;
    try {
        d = json::parse(s);
    } catch (const std::exception& e) {
        log_messages.printf(MSG_CRITICAL,
            "BUDA app: failed to parse %s: %s\n", path, e.what()
        );
        return false;
    }
    if (d.find("name") == d.end()
        || d.find("min_nsuccess") == d.end()
        || d.find("max_total") == d.end()
    ) {
        log_messages.printf(MSG_CRITICAL,
            "BUDA app: missing element in %s\n", path
        );
        return false;
    }
    name = d["name"].get<string>();
    min_nsuccess = d["min_nsuccess"].get<int>();
    max_total = d["max_total"].get<int>();

    sprintf(path, "%s/%s",
        BUDA_APPS_DIR,
        name.c_str()
    );
    DirScanner ds(path);
    string var_name;
    while (ds.scan(var_name)) {
        BUDA_VARIANT bv;
        if (bv.read_json(name, var_name)) {
            variants.push_back(bv);
        }
    }
    if (variants.empty()) {
        log_messages.printf(MSG_CRITICAL,
            "BUDA app: no variants for %s\n", name.c_str()
        );
        return false;
    }
    return true;
}

void BUDA_APPS::read_json() {
    DirScanner ds(BUDA_APPS_DIR);
    string name;
    while (ds.scan(name)) {
        BUDA_APP ba(name);
        if (ba.read_json()) {
            apps.push_back(ba);
        }
    }
}

BUDA_APP* BUDA_APPS::lookup_app(string name) {
    for (BUDA_APP& ba: apps) {
        if (ba.name == name) {
            return &ba;
        }
    }
    return NULL;
}

static int get_host_cpu_type() {
    char *p = g_request->platforms.list[0]->name;
    if (strstr(p, "x86_64")) {
        return CPU_TYPE_INTEL;
    }
    if (strstr(p, "arm")) {
        return CPU_TYPE_ARM;
    }
    return CPU_TYPE_UNKNOWN;
}

// pick best variant (if any) for a BUDA job
// resource type:
// >= 0: use only variants for that resource type
// < 0: use any variant for which work request > 0
//
// return false if none.
// else populate variant host usage
//
bool choose_buda_variant(
    WORKUNIT &wu, int resource_type, BUDA_VARIANT **bv0, HOST_USAGE &hu
) {
    string buda_app_name = get_buda_app_name(wu);
    BUDA_APP* buda_app = buda_apps.lookup_app(buda_app_name);
    if (!buda_app) {
        log_messages.printf(MSG_CRITICAL,
            "choose_buda_variant(): no BUDA app '%s'\n",
            buda_app_name.c_str()
        );
        return false;
    }
    double best_flops = 0;
    bool found = false;
    int host_cpu_type = get_host_cpu_type();
    for (BUDA_VARIANT &bv: buda_app->variants) {
        if (bv.cpu_type != host_cpu_type) {
            continue;
        }
        HOST_USAGE vhu;
        if (bv.plan_class.empty()) {
            if (resource_type < 0) {
                if (!g_wreq->need_proc_type(0)) continue;
            } else {
                if (resource_type) continue;
            }
            vhu.sequential_app(g_reply->host.p_fpops);
        } else {
            if (!app_plan(*g_request, bv.plan_class.c_str(), vhu, &wu)) {
                continue;
            }
            if (resource_type < 0) {
                if (!g_wreq->need_proc_type(vhu.proc_type)) continue;
            } else {
                if (resource_type != vhu.proc_type) continue;
            }
        }
        if (vhu.projected_flops > best_flops) {
            hu = vhu;
            *bv0 = &bv;
            best_flops = vhu.projected_flops;
            found = true;
        }
    }
    if (!found) {
        if (config.debug_send_job) {
            log_messages.printf(MSG_NORMAL,
                "[debug_send_job] no usable BUDA variant for WU %s\n",
                wu.name
            );
        }
        return false;
    }
    return true;
}

// add variant files to WU xml_doc
// initial xml_doc:
// <file_info>...</file_info>
// <workunit>
//   <file_ref>...</file_ref>
// </workunit>
//
// This modifies wu.xml_doc; only use it on a copy
//
void add_app_files(WORKUNIT &wu, BUDA_VARIANT &bv) {
    char *p1 = strstr(wu.xml_doc, "<workunit>");
    char *p2 = strstr(wu.xml_doc, "</workunit>");
    if (!p1 || !p2) {
        log_messages.printf(MSG_CRITICAL, "add_app_file(): bad xml_doc\n");
        return;
    }
    *p1 = 0;
    *p2 = 0;
    char buf[BLOB_SIZE];
    strcpy(buf, wu.xml_doc);
    strcat(buf, bv.file_infos.c_str());
    strcat(buf, "<workunit>\n");
    strcat(buf, p1+strlen("<workunit>"));
    strcat(buf, bv.file_refs.c_str());
    strcat(buf, "</workunit>\n");
    strcpy(wu.xml_doc, buf);
}

// wu is a BUDA job; get the BUDA app name from its xml_doc
//
string get_buda_app_name(WORKUNIT &wu) {
    string s = "";
    char foo[BLOB_SIZE];
    char *start = strstr(wu.xml_doc, "<buda_app_name>");
    if (!start) return string("");
    strcpy(foo, start);
    char *p = strstr(foo, "</buda_app_name>");
    if (!p) return string("");
    *p = 0;
    return string(foo+strlen("<buda_app_name>"));
}

void set_have_apps_flags() {
    for (BUDA_APP& ba: buda_apps.apps) {
        for (BUDA_VARIANT& bv: ba.variants) {
            if (!bv.plan_class.empty()) {
                int rt = plan_class_to_proc_type(bv.plan_class.c_str());
                ssp->have_apps_for_proc_type[rt] = true;
            }
        }
    }
}

void buda_init() {
    buda_apps.read_json();
    set_have_apps_flags();
}

#ifdef BUDA_TEST
int main() {
    buda_init();
}
#endif
