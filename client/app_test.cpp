// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// Set up data structures (project, app, app version, WU, result etc.)
// so that the client runs the given executable.
// Lets you debug client/app interactions with no server or fake XML files

#include "project.h"
#include "client_types.h"
#include "result.h"
#include "client_state.h"
#include "log_flags.h"

void CLIENT_STATE::app_test_init() {
    PROJECT *proj = new PROJECT;
    strcpy(proj->project_name, "test project");
    strcpy(proj->master_url, "test_project_url");
    strcpy(proj->_project_dir, ".");
    proj->app_test = true;
    projects.push_back(proj);

    APP *app = new APP;
    strcpy(app->name, "test app");
    strcpy(app->user_friendly_name, "test app");
    app->project = proj;
    // can put other stuff here like
    // app->sporadic = true;
    apps.push_back(app);

    FILE_INFO *fip = new FILE_INFO;
    strcpy(fip->name, app_test_file.c_str());
    fip->status = FILE_PRESENT;
    fip->executable = true;
    file_infos.push_back(fip);

    FILE_REF * fref = new FILE_REF;
    fref->file_info = fip;
    strcpy(fip->name, app_test_file.c_str());
    fref->main_program = true;

    APP_VERSION *av = new APP_VERSION;
    strcpy(av->app_name, "test_av");
    strcpy(av->api_version, "8.0");
    av->app = app;
    av->project = proj;
    av->app_files.push_back(*fref);
    app_versions.push_back(av);

    WORKUNIT *wu = new WORKUNIT;
    strcpy(wu->name, "test_wu");
    strcpy(wu->app_name, "test_app");
    wu->project = proj;
    wu->app = app;
    wu->rsc_fpops_est = 1e9;
    wu->rsc_fpops_bound = 1e12;
    wu->rsc_memory_bound = 1e9;
    wu->rsc_disk_bound = 1e9;
    workunits.push_back(wu);

    RESULT *res = new RESULT;
    strcpy(res->name, "test_result");
    strcpy(res->wu_name, "test_wu");
    res->project = proj;
    res->avp = av;
    res->wup = wu;
    res->app = app;
    res->report_deadline = dtime()+86400;
    results.push_back(res);

    network_suspended = true;
    cc_config.skip_cpu_benchmarks = true;
}
