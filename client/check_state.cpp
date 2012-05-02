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

// Functions to check the integrity of core client data structures.
// Not currently used, but might be handy if *0 type crashes occur

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cassert>
#endif

#include "client_msgs.h"
#include "client_state.h"
#include "result.h"

void CLIENT_STATE::check_project_pointer(PROJECT* p) {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        if (p == projects[i]) return;
    }
    assert(0);
}
void CLIENT_STATE::check_app_pointer(APP* p) {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        if (p == apps[i]) return;
    }
    assert(0);
}
void CLIENT_STATE::check_file_info_pointer(FILE_INFO* p) {
    unsigned int i;
    for (i=0; i<file_infos.size(); i++) {
        if (p == file_infos[i]) return;
    }
    assert(0);
}
void CLIENT_STATE::check_app_version_pointer(APP_VERSION* p) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        if (p == app_versions[i]) return;
    }
    assert(0);
}
void CLIENT_STATE::check_workunit_pointer(WORKUNIT* p) {
    unsigned int i;
    for (i=0; i<workunits.size(); i++) {
        if (p == workunits[i]) return;
    }
    assert(0);
}
void CLIENT_STATE::check_result_pointer(RESULT* p) {
    unsigned int i;
    for (i=0; i<results.size(); i++) {
        if (p == results[i]) return;
    }
    assert(0);
}

void CLIENT_STATE::check_pers_file_xfer_pointer(PERS_FILE_XFER* p) {
    unsigned int i;
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        if (p == pers_file_xfers->pers_file_xfers[i]) return;
    }
    assert(0);
}
void CLIENT_STATE::check_file_xfer_pointer(FILE_XFER* p) {
    unsigned int i;
    for (i=0; i<file_xfers->file_xfers.size(); i++) {
        if (p == file_xfers->file_xfers[i]) return;
    }
    assert(0);
}

void CLIENT_STATE::check_app(APP& p) {
    check_project_pointer(p.project);
}

void CLIENT_STATE::check_file_info(FILE_INFO& p) {
    if (p.pers_file_xfer) check_pers_file_xfer_pointer(p.pers_file_xfer);
    if (p.result) check_result_pointer(p.result);
    check_project_pointer(p.project);
}

void CLIENT_STATE::check_file_ref(FILE_REF& p) {
    check_file_info_pointer(p.file_info);
}

void CLIENT_STATE::check_app_version(APP_VERSION& p) {
    unsigned int i;
    check_app_pointer(p.app);
    check_project_pointer(p.project);
    for (i=0; i<p.app_files.size(); i++) {
        FILE_REF& fr = p.app_files[i];
        check_file_ref(fr);
    }
}

void CLIENT_STATE::check_workunit(WORKUNIT& p) {
    unsigned int i;
    for (i=0; i<p.input_files.size(); i++) {
        check_file_ref(p.input_files[i]);
    }
    check_project_pointer(p.project);
    check_app_pointer(p.app);
}

void CLIENT_STATE::check_result(RESULT& p) {
    unsigned int i;
    for (i=0; i<p.output_files.size(); i++) {
        check_file_ref(p.output_files[i]);
    }
    check_app_pointer(p.app);
    check_workunit_pointer(p.wup);
    check_project_pointer(p.project);
}

void CLIENT_STATE::check_active_task(ACTIVE_TASK& p) {
    check_result_pointer(p.result);
    check_workunit_pointer(p.wup);
    check_app_version_pointer(p.app_version);
}

void CLIENT_STATE::check_pers_file_xfer(PERS_FILE_XFER& p) {
    if (p.fxp) check_file_xfer_pointer(p.fxp);
    check_file_info_pointer(p.fip);
}

void CLIENT_STATE::check_file_xfer(FILE_XFER& p) {
    check_file_info_pointer(p.fip);
}

void CLIENT_STATE::check_all() {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        check_app(*apps[i]);
    }
    for (i=0; i<file_infos.size(); i++) {
        check_file_info(*file_infos[i]);
    }
    for (i=0; i<app_versions.size(); i++) {
        check_app_version(*app_versions[i]);
    }
    for (i=0; i<workunits.size(); i++) {
        check_workunit(*workunits[i]);
    }
    for (i=0; i<results.size(); i++) {
        check_result(*results[i]);
    }
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        check_active_task(*active_tasks.active_tasks[i]);
    }
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        check_pers_file_xfer(*pers_file_xfers->pers_file_xfers[i]);
    }
    for (i=0; i<file_xfers->file_xfers.size(); i++) {
        check_file_xfer(*file_xfers->file_xfers[i]);
    }
}

// Deallocate memory.  Can be used to check for memory leaks.
// Turned off for now.
//
void CLIENT_STATE::free_mem() {
    vector<PROJECT*>::iterator proj_iter;
    vector<APP*>::iterator app_iter;
    vector<FILE_INFO*>::iterator fi_iter;
    vector<APP_VERSION*>::iterator av_iter;
    vector<WORKUNIT*>::iterator wu_iter;
    vector<RESULT*>::iterator res_iter;
    PROJECT *proj;
    APP *app;
    FILE_INFO *fi;
    APP_VERSION *av;
    WORKUNIT *wu;
    RESULT *res;

    proj_iter = projects.begin();
    while (proj_iter != projects.end()) {
        proj = projects[0];
        proj_iter = projects.erase(proj_iter);
        delete proj;
    }

    app_iter = apps.begin();
    while (app_iter != apps.end()) {
        app = apps[0];
        app_iter = apps.erase(app_iter);
        delete app;
    }

    fi_iter = file_infos.begin();
    while (fi_iter != file_infos.end()) {
        fi = file_infos[0];
        fi_iter = file_infos.erase(fi_iter);
        delete fi;
    }

    av_iter = app_versions.begin();
    while (av_iter != app_versions.end()) {
        av = app_versions[0];
        av_iter = app_versions.erase(av_iter);
        delete av;
    }

    wu_iter = workunits.begin();
    while (wu_iter != workunits.end()) {
        wu = workunits[0];
        wu_iter = workunits.erase(wu_iter);
        delete wu;
    }

    res_iter = results.begin();
    while (res_iter != results.end()) {
        res = results[0];
        res_iter = results.erase(res_iter);
        delete res;
    }

    active_tasks.free_mem();

    message_descs.cleanup();
    delete http_ops;
    delete file_xfers;
    delete pers_file_xfers;
    delete scheduler_op;

    notices.clear();
    rss_feeds.clear();
    daily_xfer_history.clear();
}
