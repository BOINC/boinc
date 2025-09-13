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

#ifndef BUDA_H
#define BUDA_H

#include <vector>
#include <string>

#include "sched_types.h"

using std::vector;
using std::string;

#define CPU_TYPE_UNKNOWN    0
#define CPU_TYPE_INTEL      1
#define CPU_TYPE_ARM        2

struct BUDA_VARIANT {
    int cpu_type;
    string plan_class;
    string file_infos;      // XML for app file infos
    string file_refs;       // XML for app file refs
    bool read_json(string app_name, string var_name);
};

struct BUDA_APP {
    string name;
    int min_nsuccess;
    int max_total;
    vector<BUDA_VARIANT> variants;

    bool read_json();
    BUDA_APP(string n) {
        name = n;
        min_nsuccess = 1;
        max_total = 2;
    }
};

struct BUDA_APPS {
    vector<BUDA_APP> apps;

    void read_json();
    BUDA_APP* lookup_app(string name);
};

extern BUDA_APPS buda_apps;

inline bool is_buda(const WORKUNIT &wu) {
    return strstr(wu.xml_doc, "<buda_app_name>") != NULL;
}
extern bool choose_buda_variant(
    WORKUNIT &wu, int resource_type, BUDA_VARIANT **bv0, HOST_USAGE &hu
);
extern string get_buda_app_name(WORKUNIT &wu);
extern void add_app_files(WORKUNIT &wu, BUDA_VARIANT &bv);
extern void buda_init();

#endif
