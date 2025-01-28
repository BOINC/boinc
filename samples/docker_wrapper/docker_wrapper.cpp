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

// docker_wrapper: run a BOINC job in a Docker container,
//      and interface between the BOINC client and the container
//
// docker_wrapper [options] arg1 arg2 ...
// options:
// --verbose            write Docker commands and outputs to stderr
// --config <filename>  config filename, default job.toml
// --dockerfile         Dockerfile name, default Dockerfile
// --sporadic           app is sporadic
//
// args are passed as cmdline args to main prog in container
//
// docker_wrapper runs in a directory (usually slot dir) containing
//
// Dockerfile
// job.toml
//      optional config file
// input files (link or physical)
// executable files (link or physical)
//
// Win:
//      There must be a WSL image containing Docker or Podman
//      The wrapper runs a pipe-connected shell in WSL
//      (running in the current dir)
//      and sends commands (e.g. docker commands) via the pipe.
//
// Unix:
//      The host must have Docker or Podman
//      The wrapper runs Docker commands using popen()
//
// Logic:
// If the container already exists
//      this is a restart of the job
//      start the container if it's stopped
// else
//      this is the first run of the job
//      if the image doesn't already exist
//          build image with 'docker build'
//      (need a lock around the above?)
//      create the container with -v to mount slot, project dirs
//      copy input files as needed
// start container
// loop: handle msgs from client, check for container exit
// on successful exit
//      copy output files as needed

// Names:
// image name
//      name: lower case letters, digits, separators (. _ -); max 4096 chars
//      tag: max 128 chars
//      in the universal model, each WU must have a different image
//      so we'll use: boinc__<proj>__<wuname>
//
// container name:
//      letters, numbers, _
//      max 255 chars
//      we'll use: boinc__<proj>__<resultname>

// standalone mode (debugging):
// image name: boinc
// container name: boinc
// slot dir: .
// project dir (if any indirect files): ./project/

// enable standalone test on Win
//
//#define WIN_STANDALONE_TEST

#include <cstdio>
#include <string>
#include <vector>

#include "toml.h"
    // from https://github.com/mayah/tinytoml

#include "util.h"
#include "boinc_api.h"

#ifdef _WIN32
#include "win_util.h"
#endif

using std::string;
using std::vector;

#define POLL_PERIOD 1.0
#define STATUS_PERIOD 10
    // reports status this often

enum JOB_STATUS {JOB_IN_PROGRESS, JOB_SUCCESS, JOB_FAIL};

struct RSC_USAGE {
    double cpu_frac;
    double wss;
    void clear() {
        cpu_frac = 0;
        wss = 0;
    }
};

// parsed version of job.toml
//
struct CONFIG {
    string workdir;
        // WORKDIR in Dockerfile; default "/app"
        // slot dir will be mounted here
    string project_dir_mount;
        // mount project dir here in container
        // default: don't mount it
    void print() {
        fprintf(stderr, "Wrapper config file:\n");
        if (!workdir.empty()) {
            fprintf(stderr, "   workdir: %s\n", workdir.c_str());
        }
        if (!project_dir_mount.empty()) {
            fprintf(stderr, "   project dir mounted at: %s\n", project_dir_mount.c_str());
        }
    }
};

const char* project_dir;
char image_name[512];
char container_name[512];
APP_INIT_DATA aid;
CONFIG config;
bool running;
bool verbose = false;
const char* config_file = "job.toml";
const char* dockerfile = "Dockerfile";
DOCKER_CONN docker_conn;
vector<string> app_args;

// parse job config file
//
int parse_config_file() {
    // defaults
    config.workdir = "/app";

    std::ifstream ifs(config_file);
    if (ifs.fail()) {
        fprintf(stderr, "no job.toml config file\n");
        return 0;
    }
    toml::ParseResult r = toml::parse(ifs);
    if (!r.valid()) {
        fprintf(stderr, "TOML error: %s\n", r.errorReason.c_str());
        return 1;
    }
    const toml::Value &v = r.value;
    const toml::Value *x;
    x = v.find("workdir");
    if (x) {
        config.workdir = x->as<string>();
    }
    x = v.find("project_dir_mount");
    if (x) {
        config.project_dir_mount = x->as<string>();
    }
    return 0;
}

// See if command output includes "Error"
//
int error_output(vector<string> &out) {
    for (string line: out) {
        if (strstr(line.c_str(), "Error")) {
            return -1;
        }
    }
    return 0;
}

//////////  IMAGE  ////////////

void get_image_name() {
    string s = docker_image_name(project_dir, aid.wu_name);
    strcpy(image_name, s.c_str());
}

int image_exists(bool &exists) {
    vector<string> out;

    int retval = docker_conn.command("images", out);
    if (retval) return retval;
    string image_name_space = image_name + string(" ");
    for (string line: out) {
        if (line.find(image_name_space) != string::npos) {
            exists = true;
            return 0;
        }
    }
    exists = false;
    return 0;
}

int build_image() {
    char cmd[256];
    vector<string> out;
    sprintf(cmd, "build . -t %s -f %s", image_name, dockerfile);
    int retval = docker_conn.command(cmd, out);
    if (retval) return retval;
    return 0;
}

// build image if needed
//
int get_image() {
    bool exists;
    int retval;

    retval = image_exists(exists);
    if (retval) {
        fprintf(stderr, "image_exists() failed: %d\n", retval);
        exit(1);
    }
    if (!exists) {
        if (verbose) fprintf(stderr, "building image\n");
        retval = build_image();
        if (retval) {
            fprintf(stderr, "build_image() failed: %d\n", retval);
            exit(1);
        }
    }
    return 0;
}

//////////  CONTAINER  ////////////

void get_container_name() {
    string s = docker_container_name(project_dir, aid.result_name);
    strcpy(container_name, s.c_str());
}

int container_exists(bool &exists) {
    char cmd[1024];
    int retval;
    vector<string> out;

    sprintf(cmd, "ps --all --filter \"name=%s\"", container_name);
    retval = docker_conn.command(cmd, out);
    if (retval) return retval;
    for (string line: out) {
        if (strstr(line.c_str(), container_name)) {
            exists = true;
            return 0;
        }
    }
    exists = false;
    return 0;
}

int create_container() {
    char cmd[1024];
    char slot_cmd[256], project_cmd[256];
    vector<string> out;
    int retval;

    retval = get_image();
    if (retval) return retval;

    sprintf(slot_cmd, " -v .:%s",
        config.workdir.c_str()
    );
    if (config.project_dir_mount.empty()) {
        project_cmd[0] = 0;
    } else {
        if (boinc_is_standalone()) {
            sprintf(project_cmd, " -v %s:%s",
                project_dir, config.project_dir_mount.c_str()
            );
        } else {
            sprintf(project_cmd, " -v ../../projects/%s:%s",
                project_dir, config.project_dir_mount.c_str()
            );
        }
    }
    sprintf(cmd, "create --name %s %s %s",
        container_name,
        slot_cmd, project_cmd
    );
    // add command-line args
    strcat(cmd, " -e ARGS=\"");
    for (string arg: app_args) {
        strcat(cmd, " ");
        strcat(cmd, arg.c_str());
    }
    strcat(cmd, "\" ");
    strcat(cmd, image_name);
    retval = docker_conn.command(cmd, out);
    if (retval) return retval;
    if (error_output(out)) return -1;

    return 0;
}

//////////  JOB CONTROL  ////////////

int container_op(const char *op) {
    char cmd[1024];
    vector<string> out;
    sprintf(cmd, "%s %s", op, container_name);
    int retval = docker_conn.command(cmd, out);
    return retval;
}

// Clean up at end of job.
// Show log output.
// remove container and image
//
void cleanup() {
    char cmd[1024];
    vector<string> out;

    sprintf(cmd, "logs %s", container_name);
    docker_conn.command(cmd, out);
    fprintf(stderr, "stderr from container:\n");
    for (string line : out) {
        fprintf(stderr, "%s\n", line.c_str());
    }
    fprintf(stderr, "stderr end\n");

    container_op("stop");

    sprintf(cmd, "container rm %s", container_name);
    docker_conn.command(cmd, out);

    sprintf(cmd, "image rm %s", image_name);
    docker_conn.command(cmd, out);
}

void poll_client_msgs() {
    BOINC_STATUS status;
    boinc_get_status(&status);
    if (status.no_heartbeat || status.quit_request || status.abort_request) {
        fprintf(stderr, "got quit/abort from client\n");
        container_op("stop");
        exit(0);
    }
    if (status.suspended) {
        if (verbose) {
            fprintf(stderr, "client: suspended\n");
        }
        if (running) {
            container_op("pause");
            running = false;
        }
    } else {
        if (verbose) {
            fprintf(stderr, "client: not suspended\n");
        }
        if (!running) {
            container_op("unpause");
            running = true;
        }
    }
}

// check whether job has exited
// Note: on both Podman and Docker this takes significant CPU time
// (like .03 sec) so do it infrequently (like 5 sec)
//
JOB_STATUS poll_app() {
    char cmd[1024];
    vector<string> out;
    int retval;

    sprintf(cmd, "ps --all -f \"name=%s\"", container_name);
    retval = docker_conn.command(cmd, out);
    if (retval) return JOB_FAIL;
    for (string line: out) {
        if (strstr(line.c_str(), container_name)) {
            if (strstr(line.c_str(), "Exited")) {
                return JOB_SUCCESS;
            }
            return JOB_IN_PROGRESS;
        }
    }
    return JOB_FAIL;
}

// get CPU and mem usage
// This is also surprisingly slow
int get_stats(RSC_USAGE &ru) {
    char cmd[1024];
    vector<string> out;
    int retval;
    size_t n;

    sprintf(cmd,
        "stats --no-stream  --format \"{{.CPUPerc}} {{.MemUsage}}\" %s",
        container_name
    );
    retval = docker_conn.command(cmd, out);
    if (retval) return -1;
    if (out.empty()) return -1;
    const char *buf = out[0].c_str();
    // output is like
    // 0.00% 420KiB / 503.8GiB
    double cpu_pct, mem;
    char mem_unit;
    n = sscanf(buf, "%lf%% %lf%c", &cpu_pct, &mem, &mem_unit);
    if (n != 3) return -1;
    switch (mem_unit) {
    case 'G':
    case 'g':
        mem *= GIGA; break;
    case 'M':
    case 'm':
        mem *= MEGA; break;
    case 'K':
    case 'k':
        mem *= KILO; break;
    case 'B':
    case 'b':
        break;
    default: return -1;
    }
    ru.cpu_frac = cpu_pct/100.;
    ru.wss = mem;
    return 0;
}

#ifdef _WIN32
// find a WSL distro with Docker and set up a command link to it
//
int wsl_init() {
    string distro_name;
    DOCKER_TYPE docker_type;
    if (boinc_is_standalone()) {
        distro_name = "Ubuntu";
        docker_type = PODMAN;
    } else {
        WSL_DISTRO* distro = aid.host_info.wsl_distros.find_docker();
        if (!distro) return -1;
        distro_name = distro->distro_name;
        docker_type = distro->docker_type;
    }
    return docker_conn.init(docker_type, distro_name, verbose);
}
#endif

int main(int argc, char** argv) {
    BOINC_OPTIONS options;
    int retval;
    bool sporadic = false, exists;
    RSC_USAGE ru;

    for (int j=1; j<argc; j++) {
        if (!strcmp(argv[j], "--sporadic")) {
            sporadic = true;
        } else if (!strcmp(argv[j], "--verbose")) {
            verbose = true;
        } else if (!strcmp(argv[j], "--config")) {
            config_file = argv[++j];
        } else if (!strcmp(argv[j], "--dockerfile")) {
            dockerfile = argv[++j];
        } else {
            app_args.push_back(argv[j]);
        }
    }

#ifdef WIN_STANDALONE_TEST
    SetCurrentDirectoryA("C:/ProgramData/BOINC/slots/test_docker");
#endif

    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;
    boinc_init_options(&options);

    if (boinc_is_standalone()) {
        verbose = true;
        strcpy(image_name, "boinc");
        strcpy(container_name, "boinc");
        project_dir = "project";
    } else {
        boinc_get_init_data(aid);
        project_dir = strrchr(aid.project_dir, '/')+1;
        get_image_name();
        get_container_name();
    }
    retval = parse_config_file();
    if (retval) {
        fprintf(stderr, "can't parse config file\n");
        exit(1);
    }
    if (verbose) config.print();

    if (sporadic) {
        retval = boinc_sporadic_dir(".");
        if (retval) {
            fprintf(stderr, "can't create sporadic files\n");
            boinc_finish(retval);
        }
    }

#ifdef _WIN32
    retval = wsl_init();
    if (retval) {
        fprintf(stderr, "wsl_init() failed: %d\n", retval);
        boinc_finish(1);
    }
#else
    docker_conn.init(
        boinc_is_standalone()?DOCKER:aid.host_info.docker_type, verbose
    );
#endif

    retval = container_exists(exists);
    if (retval) {
        fprintf(stderr, "container_exists() failed: %d\n", retval);
        boinc_finish(1);
    }
    if (!exists) {
        if (verbose) fprintf(stderr, "creating container %s\n", container_name);
        retval = create_container();
        if (retval) {
            fprintf(stderr, "create_container() failed: %d\n", retval);
            boinc_finish(1);
        }
    }
    if (verbose) fprintf(stderr, "starting container\n");
    retval = container_op("start");
    if (retval) {
        fprintf(stderr, "resume() failed: %d\n", retval);
        cleanup();
        boinc_finish(1);
    }
    running = true;
    double cpu_time = 0;
    for (int i=0; ; i++) {
        poll_client_msgs();
        if (i%STATUS_PERIOD == 0) {
            switch(poll_app()) {
            case JOB_FAIL:
                cleanup();
                boinc_finish(1);
                break;
            case JOB_SUCCESS:
                cleanup();
                boinc_finish(0);
                break;
            default:
                break;
            }
            retval = get_stats(ru);
            if (!retval) {
                cpu_time += STATUS_PERIOD*ru.cpu_frac;
                if (verbose) {
                    fprintf(stderr, "reporting CPU %f WSS %f\n", cpu_time, ru.wss);
                }
                boinc_report_app_status_aux(
                    cpu_time,
                    0,      // checkpoint CPU time
                    0,      // frac done
                    0,      // other PID
                    0,0,    // bytes send/received
                    ru.wss
                );
            }
        }
        boinc_sleep(POLL_PERIOD);
    }
}
