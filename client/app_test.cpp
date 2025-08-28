// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// This framework lets you run jobs under a BOINC client without a project.
// This lets you debug client/app interactions like suspend/resume.
// The properties of the app and its files
// are described procedurally (in this file)
// rather than with XML files.
//
// To use this framework:
// - edit this file to describe your application:
//      input/output files, attributes, etc.
//      It currently has several test cases, selected with #ifdef
// - build the BOINC client with these changes
// - Linux: make a BOINC data directory, say 'test'
//      (or you can use an existing BOINC data directory,
//      in which case the client will also run existing jobs)
// - make a directory test/slots/app_test
//      The client will run your test job there.
//      Clean it out between runs.
// - make a dir test/projects/app_test
// - In the project directory, put:
//      - the executable file
//      - the input file(s) with physical names
//      NOTE: slots/app_test and projects/app_test can be symbolic links
//      in case you have multiple test cases
// - run boinc in the data directory, e.g. test/
//      The client will copy files and create link files
//      as needed in the slot dir,
//      and create init_data.xml there.
//      When the job is done, the client won't clean out the slot dir.
//      You can examine the contents of the slot and project dir,
//      Clean out the slot dir between tests.

#include "client_state.h"

// define exactly one

#define APP_NONE
//#define APP_WSL_WRAPPER
//      type    physical            logical             copy?
//      app     wsl_wrapper.exe     wsl_wrapper.exe
//      app     worker              worker
//      app     main                main                yes
//      input   infile              in
//      output  outfile             out
//#define APP_DOCKER_WRAPPER
//      type    physical            logical             copy?
//      app     worker              worker              yes
//      app     job.toml            job.toml            yes
//	(not used for now)
//      app     Dockerfile          Dockerfile          yes
//      app     main.sh             main.sh             yes
//      app     docker_wrapper      docker_wrapper
//      input   in	            in                  yes
//      output  out                 out                 yes

#ifdef APP_NONE
void CLIENT_STATE::app_test_init() {}
#else

#include "client_types.h"
#include "log_flags.h"
#include "project.h"
#include "result.h"

// The following functions create client data structures
// (PROJECT, APP, APP_VERSION, WORKUNIT, RESULT, FILE_REF, FILE_INFO)
// for the test app and job.
// The names and version numbers must match up.

static PROJECT* make_project() {
    PROJECT *proj = new PROJECT;
    strcpy(proj->project_name, "app_test project");
    strcpy(proj->master_url, "https://app.test/");
    strcpy(proj->_project_dir, "projects/app_test");
    proj->app_test = true;
        // tell the client to use the slots/app_test slot dir for this project
    gstate.projects.push_back(proj);
    return proj;
}

static APP* make_app(PROJECT* proj) {
    APP *app = new APP;
    strcpy(app->name, "test_app");
    strcpy(app->user_friendly_name, "test_app");
    app->project = proj;
    gstate.apps.push_back(app);
    return app;
}

#define INPUT_FILE  0
#define OUTPUT_FILE 1
#define MAIN_PROG   2

// if log_name is NULL, logical name is physical name
//
static FILE_REF* make_file(
    PROJECT *proj, const char* phys_name, const char* log_name,
    int ftype, bool copy_file
) {
    FILE_INFO *fip = new FILE_INFO;
    strcpy(fip->name, phys_name);
    fip->project = proj;
    fip->status = (ftype == OUTPUT_FILE)?FILE_NOT_PRESENT:FILE_PRESENT;
    fip->executable = true;
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
    if (ftype == MAIN_PROG) fref->main_program = true;
    fref->copy_file = copy_file;
    return fref;
}

static APP_VERSION* make_app_version(APP *app) {
    APP_VERSION *av = new APP_VERSION;
    strcpy(av->app_name, app->name);
    strcpy(av->api_version, "8.0");
    av->app = app;
    av->project = app->project;
    av->resource_usage.avg_ncpus = 1;
    av->version_num = 1;
    av->resource_usage.flops = 1e9;
    gstate.app_versions.push_back(av);
    return av;
}

static WORKUNIT* make_workunit(APP_VERSION *av) {
    WORKUNIT *wu = new WORKUNIT;
    APP* app = av->app;
    strcpy(wu->name, "test_wu");
    strcpy(wu->app_name, app->name);
    wu->app = app;
    wu->project = app->project;
    wu->rsc_fpops_est = 1e9;
    wu->rsc_fpops_bound = 1e12;
    wu->rsc_memory_bound = 1e9;
    wu->rsc_disk_bound = 1e9;
    wu->version_num = av->version_num;
    gstate.workunits.push_back(wu);
    return wu;
}

static RESULT* make_result(APP_VERSION *av, WORKUNIT* wu) {
    RESULT *res = new RESULT;
    strcpy(res->name, "test_result");
    strcpy(res->wu_name, wu->name);
    res->project = av->project;
    res->avp = av;
    res->wup = wu;
    res->app = av->app;
    res->report_deadline = dtime()+86400;
    res->_state = RESULT_FILES_DOWNLOADED;
    gstate.results.push_back(res);
    return res;
}

// app_test_init() sets up above data structures
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

    APP_VERSION *av = make_app_version(app);

////////////// APP VERSION AND WORKUNIT FILES /////////////////

#ifdef APP_WSL_WRAPPER
    av->app_files.push_back(
        *make_file(app->project, "wsl_wrapper.exe", NULL, MAIN_PROG, false)
    );
    av->app_files.push_back(
        *make_file(app->project, "main", NULL, INPUT_FILE, true)
    );
    av->app_files.push_back(
        *make_file(app->project, "worker", NULL, INPUT_FILE, false)
    );
#endif
#ifdef APP_DOCKER_WRAPPER
    av->app_files.push_back(
        *make_file(app->project, "docker_wrapper", NULL, MAIN_PROG, false)
    );
    av->app_files.push_back(
        *make_file(app->project, "worker", NULL, INPUT_FILE, true)
    );
    av->app_files.push_back(
        *make_file(app->project, "main.sh", "main.sh", INPUT_FILE, true)
    );
#if 0
    av->app_files.push_back(
        *make_file(app->project, "job.toml", "job.toml", INPUT_FILE, true)
    );
#endif
    av->app_files.push_back(
        *make_file(app->project, "Dockerfile", "Dockerfile", INPUT_FILE, true)
    );
#endif

    // can put other stuff here like
#if 0
    av->gpu_ram = 1e7;
    av->gpu_usage.rsc_type = PROC_TYPE_NVIDIA_GPU;
    av->gpu_usage.usage = 1;
#endif

    WORKUNIT *wu = make_workunit(av);
    char buf[256];
    getcwd(buf, sizeof(buf));

////////////// INPUT FILES /////////////////

#ifdef APP_WSL_WRAPPER
    //wu->command_line = "--nsecs 60";
    wu->input_files.push_back(
        *make_file(proj, "infile", "in", INPUT_FILE, false)
    );
#endif
#ifdef APP_DOCKER_WRAPPER
    wu->command_line = "--verbose --nsecs 20";
    wu->input_files.push_back(
        *make_file(proj, "in", "in", INPUT_FILE, true)
    );
#endif
    RESULT *result = make_result(av, wu);

////////////// OUTPUT FILES /////////////////

#ifdef APP_WSL_WRAPPER
    result->output_files.push_back(
        *make_file(proj, "outfile", "out", OUTPUT_FILE, false)
    );
#endif
#ifdef APP_DOCKER_WRAPPER
    result->output_files.push_back(
        *make_file(proj, "out", "out", OUTPUT_FILE, true)
    );
#endif

    // tell the client not to get work or run benchmarks
    //
    cc_config.unsigned_apps_ok = true;
    cc_config.skip_cpu_benchmarks = true;
}
#endif
