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

// A framework that lets you run jobs under a BOINC client
// without a project, and without fake XML files
// Lets you debug client/app interactions.
//
// To use this framework:
// edit this file to describe your application:
//      input/output files, attributes, etc.
//      NOTE: currently it's set up for the 'uc2' app,
//      which reads 'in' and writes 'out' (logical names).
//      The job uses physical names 'infile' and 'outfile'.
// build the BOINC client
// make a 'test' directory
//      (or you can use an existing BOINC data directory,
//      in which case the client will also run jobs that are there)
// make a directory test/slots/app_test
//      The client will run the test job there.
//      Clean it out between runs.
// put the executable file and input file(s) in the test directory
//      (which acts as the project directory)
// in the test directory, run boinc --app_test
//      when the job is done, the client won't clean out the slot dir.
//      You can examine the contents of the slot dir,
//      and examine the output files in the test dir.

#include "project.h"
#include "client_types.h"
#include "result.h"
#include "client_state.h"
#include "log_flags.h"

static PROJECT* make_project() {
    PROJECT *proj = new PROJECT;
    strcpy(proj->project_name, "test project");
    strcpy(proj->master_url, "test_project_url");
    strcpy(proj->_project_dir, ".");
    proj->app_test = true;
    proj->non_cpu_intensive = false;
    gstate.projects.push_back(proj);
    return proj;
}

static APP* make_app(PROJECT* proj) {
    APP *app = new APP;
    strcpy(app->name, "test app");
    strcpy(app->user_friendly_name, "test app");
    app->project = proj;
    gstate.apps.push_back(app);
    return app;
}

#define INPUT_FILE  0
#define OUTPUT_FILE 1
#define EXEC_FILE   2

static FILE_REF* make_file(
    PROJECT *proj, const char* phys_name, const char* log_name, int ftype
) {
    FILE_INFO *fip = new FILE_INFO;
    strcpy(fip->name, phys_name);
    fip->project = proj;
    fip->status = (ftype == OUTPUT_FILE)?FILE_NOT_PRESENT:FILE_PRESENT;
    if (ftype == EXEC_FILE) fip->executable = true;
    if (ftype == OUTPUT_FILE) {
        fip->max_nbytes = 1e9;
        fip->upload_urls.add(string("foobar"));
    }
    gstate.file_infos.push_back(fip);
    FILE_REF * fref = new FILE_REF;
    if (log_name) {
        strcpy(fref->open_name, log_name);
    }
    fref->file_info = fip;
    if (ftype == EXEC_FILE) fref->main_program = true;
    return fref;
}

static APP_VERSION* make_app_version(APP *app, const char* exec_name) {
    APP_VERSION *av = new APP_VERSION;
    strcpy(av->app_name, "test_av");
    strcpy(av->api_version, "8.0");
    av->app = app;
    av->project = app->project;
    av->avg_ncpus = 1;
    av->flops = 1e9;
    FILE_REF *fref = make_file(app->project, exec_name, NULL, EXEC_FILE);
    av->app_files.push_back(*fref);
    gstate.app_versions.push_back(av);
    return av;
}

static WORKUNIT* make_workunit(APP* app) {
    WORKUNIT *wu = new WORKUNIT;
    strcpy(wu->name, "test_wu");
    strcpy(wu->app_name, "test_app");
    wu->app = app;
    wu->project = app->project;
    wu->rsc_fpops_est = 1e9;
    wu->rsc_fpops_bound = 1e12;
    wu->rsc_memory_bound = 1e9;
    wu->rsc_disk_bound = 1e9;
    gstate.workunits.push_back(wu);
    return wu;
}

static RESULT* make_result(APP_VERSION *av, WORKUNIT* wu) {
    RESULT *res = new RESULT;
    strcpy(res->name, "test_result");
    strcpy(res->wu_name, "test_wu");
    res->project = av->project;
    res->avp = av;
    res->wup = wu;
    res->app = av->app;
    res->report_deadline = dtime()+86400;
    gstate.results.push_back(res);
    return res;
}

// app_test_init() sets up data structures
// (project, app, app version, WU, result)
// so that the client runs a test job.
//
void CLIENT_STATE::app_test_init() {
    PROJECT *proj = make_project();

    APP *app = make_app(proj);
    // can put other stuff here like
#if 0
    app->sporadic = true;
    have_sporadic_app = true;
#endif

    APP_VERSION *av = make_app_version(app, app_test_file.c_str());
    // can put other stuff here like
#if 0
    av->gpu_ram = 1e7;
    av->gpu_usage.rsc_type = PROC_TYPE_NVIDIA_GPU;
    av->gpu_usage.usage = 1;
#endif

    WORKUNIT *wu = make_workunit(app);
#if 1
    wu->command_line = "in out";
    wu->input_files.push_back(
        *make_file(proj, "infile", "in", INPUT_FILE)
    );
#endif

    RESULT *result = make_result(av, wu);
#if 1
    result->output_files.push_back(
        *make_file(proj, "outfile", "out", OUTPUT_FILE)
    );
#endif

    // tell the client not to get work or run benchmarks
    //
    cc_config.unsigned_apps_ok = true;
    cc_config.skip_cpu_benchmarks = true;
}
