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
// See https://github.com/BOINC/boinc/wiki/Docker-apps#the-docker-wrapper
//
// docker_wrapper [options] arg1 arg2 ...
// options:
// --verbose            write non-polling Docker commands and outputs to stderr
// --config <filename>  config filename, default job.toml
// --dockerfile         Dockerfile name, default Dockerfile
// --sporadic           app is sporadic
//
// additional args are passed as cmdline args to main prog in container
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
// Startup logic:
// If the container already exists
//      this is a restart of the job
//      start the container if it's stopped,
//      unpause if it's paused
// else
//      this is the first run of the job
//      if the image doesn't already exist
//          build image with 'docker build'
//      (need a lock around the above?)
//      create the container with -v to mount slot, project dirs
//      copy input files as needed
// start container
//
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

// docs:
// https://docs.docker.com/reference/cli/docker/container/run/
// https://docs.docker.com/engine/containers/resource_constraints/

// ************** Checkpoint/restart ***************
//
// This would limit lost work if the host is rebooted
// while a BUDA job is running.
// In other cases (e.g. the user exits the client)
// we pause the container and no work is lost.
//
// Podman and Docker have different interfaces:
// podman: https://podman.io/docs/checkpoint
//      podman container checkpoint <name>
//      podman container restore <name>
// docker: https://docs.docker.com/reference/cli/docker/checkpoint/
//      docker checkpoint create <cont_name> <checkpoint_name>
//      docker checkpoint ls   (lists checkpoints)
//      docker checkpoint rm   (delete checkpoint)
//      docker start --checkpoint <checkpoint_name> <cont_name>
//      (use <cont_name>_checkpoint as the checkpoint name)
//      need --security-opt=seccomp:unconfined in initial run command?
//
// In both cases the CRIU library is required.
//
// Note: I spent several days trying to get it to work on Podman
// with all sorts of variants like --pre-checkpoint, --keep etc.
// Whenever I did
// - create a checkpoint
// - compute some more
// - stop or kill the container
// - restore
// I always get
// Error: OCI runtime error: runc: criu failed: type RESTORE errno 0
//
// So I gave up; checkpoint/restart is only relevant to the reboot scenario,
// this is rare.
// If we ever figure this out, uncomment the following

//#define CHECKPOINT_RESTART

#include <cstdio>
#include <string>
#include <vector>
#include <signal.h>

#include "toml.h"
    // from https://github.com/mayah/tinytoml

#include "util.h"
#include "str_replace.h"
#include "str_util.h"
#include "boinc_api.h"
#include "network.h"
#include "version.h"

#ifdef _WIN32
#include "win_util.h"
#endif

using std::string;
using std::vector;

#define POLL_PERIOD 1.0
#define STATUS_PERIOD 10
    // reports status this often
#define MIN_CHECKPOINT_INTERVAL 900
    // checkpoint at most every 15 min
#define CHECKPOINT_INTERVAL_FACTOR  100
    // if checkpoint takes X sec, don't do another one for X*100 sec
#define FRACTION_DONE_FILENAME "fraction_done"
#define CHECKPOINT_FILENAME "checkpoint_time"
    // on successful checkpoint, write how long it took to this file

int container_exit_code = 0;
enum JOB_STATUS {JOB_IN_PROGRESS, JOB_SUCCESS, JOB_FAIL};

struct RSC_USAGE {
    double cpu_frac;
    double wss;
    void clear() {
        cpu_frac = 0;
        wss = 0;
    }
};

// verbosity levels
#define VERBOSE_STD     1
    // include only start/pause/end commands
#define VERBOSE_ALL     2
    // include poll commands as well

// parsed version of job.toml
//
struct CONFIG {
    string workdir;
        // WORKDIR in Dockerfile; default "/app"
        // slot dir will be mounted here
    string project_dir_mount;
        // mount project dir here in container
        // default: don't mount it
    string image_name;
        // use this as the image name, and don't delete it when done.
        // For testing.
    string build_args;
        // additional args for docker build command
    string create_args;
        // additional args for docker create command
    int verbose;
    bool use_gpu;
        // tell Docker to enable GPU access
    int web_graphics_guest_port;
        // map this to a host port,
        // and inform the client using boinc_web_graphics_url()
    vector<string> mounts;
        // -v args in create container
    vector<string> portmaps;
        // -p args in create container
    void print() {
        fprintf(stderr, "docker_wrapper config:\n");
        if (!workdir.empty()) {
            fprintf(stderr, "   workdir: %s\n", workdir.c_str());
        }
        if (!project_dir_mount.empty()) {
            fprintf(stderr, "   project dir mounted at: %s\n", project_dir_mount.c_str());
        }
        fprintf(stderr, "   use GPU: %s\n", use_gpu?"yes":"no");
        if (web_graphics_guest_port) {
            fprintf(stderr, "   Web graphics guest port: %d\n",
                web_graphics_guest_port
            );
        }
        for (string& s: mounts) {
            fprintf(stderr, "   mount: %s\n", s.c_str());
        }
        for (string& s: portmaps) {
            fprintf(stderr, "   portmap: %s\n", s.c_str());
        }
        if (!build_args.empty()) {
            fprintf(stderr, "   build args: %s\n", build_args.c_str());
        }
        if (!create_args.empty()) {
            fprintf(stderr, "   create args: %s\n", create_args.c_str());
        }
        fprintf(stderr, "   verbose: %d\n", verbose);
    }
};

const char* project_dir;
char image_name[512];
char container_name[512];
APP_INIT_DATA aid;
CONFIG config;
bool running;
const char* config_file = "job.toml";
const char* dockerfile = "Dockerfile";
DOCKER_CONN docker_conn;
vector<string> app_args;
DOCKER_TYPE docker_type;
string wsl_distro_name;
double cpu_time = 0;
double checkpoint_cpu_time = 0;

#ifdef CHECKPOINT_RESTART
bool checkpoint_failed = false;
double last_checkpoint_dur = 0;
double min_checkpoint_time = 0;
#endif

// parse job config file (job.toml)
//
int parse_config_file() {
    // defaults
    config.workdir = "/app";
    config.use_gpu = false;

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

    x = v.find("image_name");
    if (x) {
        config.image_name = x->as<string>();
    }

    x = v.find("use_gpu");
    if (x) {
        config.use_gpu = x->as<bool>();
    }

    x = v.find("web_graphics_guest_port");
    if (x) {
        config.web_graphics_guest_port = x->as<int>();
    }

    x = v.find("mount");
    if (x) {
        const toml::Array& ar = x->as<toml::Array>();
        for (const toml::Value& val: ar) {
            string s = val.as<string>();
            config.mounts.push_back(s);
        }
    }
    x = v.find("portmap");
    if (x) {
        const toml::Array& ar = x->as<toml::Array>();
        for (const toml::Value& val: ar) {
            string s = val.as<string>();
            config.portmaps.push_back(s);
        }
    }
    x = v.find("build_args");
    if (x) {
        config.build_args = x->as<string>();
    }
    x = v.find("create_args");
    if (x) {
        config.create_args = x->as<string>();
    }
    x = v.find("verbose");
    if (x) {
        config.verbose = x->as<int>();
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

//////////  PODMAN ARGS  ////////////

#ifdef __APPLE__
// podman doesn't correctly handle args containing unescaped spaces - WTF??
//
static void escape_spaces(const char* p, char *q) {
    while (*p) {
        if (*p == ' ') {
            *q++ = '\\';
            *q++ = ' ';
        } else {
            *q++ = *p;
        }
        p++;
    }
    *q = 0;
}
#endif

char escaped_cwd[MAXPATHLEN];

void get_escaped_cwd() {
    // on MacOS/podman, you need the full path, not .
    // Win: use . since full path has :
    //
#ifdef __APPLE__
    char cwd2[MAXPATHLEN];
    getcwd(cwd2, sizeof(cwd2));
    escape_spaces(cwd2, escaped_cwd);
#else
    strcpy(escaped_cwd, ".");
#endif
}

void get_app_args(char* buf) {
    buf[0] = 0;
#ifdef __APPLE__
    for (string arg: app_args) {
        char buf2[4096];
        strcat(buf, "\\ ");
        escape_spaces(arg.c_str(), buf2);
        strcat(buf, buf2);
    }
#else
    for (string arg: app_args) {
        strcat(buf, " ");
        strcat(buf, arg.c_str());
    }
#endif
}

//////////  IMAGE  ////////////

void get_image_name() {
    if (config.image_name.empty()) {
        string s = docker_image_name(project_dir, aid.wu_name);
        safe_strcpy(image_name, s.c_str());
    } else {
        safe_strcpy(image_name, config.image_name.c_str());
    }
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
    snprintf(cmd, sizeof(cmd), "build \"%s\" -t %s -f %s %s",
        escaped_cwd, image_name, dockerfile, config.build_args.c_str()
    );
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
        if (config.verbose) fprintf(stderr, "building image\n");
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
    safe_strcpy(container_name, s.c_str());
}

#define CONTAINER_ABSENT    1
#define CONTAINER_CREATED   2
#define CONTAINER_RUNNING   3
#define CONTAINER_PAUSED    4
#define CONTAINER_EXITED    5
#define CONTAINER_OTHER     6

int get_container_state(int &state) {
    char cmd[1024];
    int retval;
    vector<string> out;

    snprintf(cmd, sizeof(cmd),
        "ps --all --filter \"name=^%s$\" --format \"{{.Names}}|{{.Status}}\"",

        container_name
    );
    retval = docker_conn.command(cmd, out);
    if (retval) return retval;
    for (string line: out) {
        char buf[256];
        safe_strcpy(buf, line.c_str());
        if (strstr(buf, container_name)) {
            char *p = strchr(buf, '|');
            if (!p) break;
            p++;
            fprintf(stderr, "container state: %s\n", p);
            if (strcasestr(p, "created")) {
                state = CONTAINER_CREATED;
            } else if (strcasestr(p, "running")) {
                state = CONTAINER_RUNNING;
            } else if (strcasestr(p, "paused")) {
                state = CONTAINER_PAUSED;
            } else if (strcasestr(p, "exited")) {
                state = CONTAINER_EXITED;
            } else {
                state = CONTAINER_OTHER;
            }
            return 0;
        }
    }
    state = CONTAINER_ABSENT;
    return 0;
}

int create_container() {
    char cmd[1024];
    char slot_cmd[256], project_cmd[256], buf[256];
    vector<string> out;
    int retval;

    retval = get_image();
    if (retval) return retval;

    // mount slot dir at /app (or whatever is specified)
    // Needs quotes since paths can contain spaces
    //
    snprintf(slot_cmd, sizeof(slot_cmd),
        " -v \"%s/\":\"%s\"",
        escaped_cwd, config.workdir.c_str()
    );
    if (config.project_dir_mount.empty()) {
        project_cmd[0] = 0;
    } else {
        if (boinc_is_standalone()) {
            snprintf(project_cmd, sizeof(project_cmd),
                " -v \"%s/%s/\":\"%s\"",
                escaped_cwd, project_dir, config.project_dir_mount.c_str()
            );
        } else {
            snprintf(project_cmd, sizeof(project_cmd),
                " -v \"%s/../../projects/%s/\":\"%s\"",
                escaped_cwd, project_dir, config.project_dir_mount.c_str()
            );
        }
    }
    snprintf(cmd, sizeof(cmd), "create --name %s %s %s",
        container_name,
        slot_cmd, project_cmd
    );

    // add command-line args, space-escaped if needed (Mac)
    //
    if (app_args.size()) {
        char arg_buf[4096];
        strcat(cmd, " -e ARGS=\"");
        get_app_args(arg_buf);
        strcat(cmd, arg_buf);
        strcat(cmd, "\"");
    }

    // add mounts and portmaps
    //
    for (string s: config.mounts) {
        snprintf(buf, sizeof(buf), " -v %s", s.c_str());
        strcat(cmd, buf);
    }
    for (string s: config.portmaps) {
        snprintf(buf, sizeof(buf), " -p %s", s.c_str());
        strcat(cmd, buf);
    }

    // multithread
    //
    if (aid.ncpus > 1) {
        snprintf(buf, sizeof(buf), " --cpus %f", aid.ncpus);
        strcat(cmd, buf);
    }

    // GPU access
    //
    if (config.use_gpu) {
        strcat(cmd, " --gpus all");
    }

    // web graphics
    //
    if (config.web_graphics_guest_port) {
        int host_port = 0;
        retval = boinc_get_port(false, host_port);
        if (retval) {
            fprintf(stderr, "can't allocate host port for web graphics: %s\n",
                boincerror(retval)
            );
        } else {
            fprintf(stderr, "web graphics: host port %d, guest port %d\n",
                host_port, config.web_graphics_guest_port
            );
            snprintf(buf, sizeof(buf), " -p %d:%d",
                host_port, config.web_graphics_guest_port
            );
            strcat(cmd, buf);
            snprintf(buf, sizeof(buf), "http://localhost:%d", host_port);
            boinc_web_graphics_url(buf);
        }
    }

    if (!config.create_args.empty()) {
        strcat(cmd, " ");
        strcat(cmd, config.create_args.c_str());
    }

    // podman sends logging to journald otherwise
    //
    if (docker_type == PODMAN) {
        strcat(cmd, " --log-driver=k8s-file");
    }

    strcat(cmd, " ");
    strcat(cmd, image_name);
    retval = docker_conn.command(cmd, out);
    if (retval) {
        fprintf(stderr, "create command failed: %d\n", retval);
        return retval;
    }
    if (error_output(out)) {
        fprintf(stderr, "create command output contains 'Error'\n");
        return -1;
    }

    return 0;
}

//////////  JOB CONTROL  ////////////

int container_op(const char *op) {
    char cmd[1024];
    vector<string> out;
    snprintf(cmd, sizeof(cmd), "%s %s", op, container_name);
    int retval = docker_conn.command(cmd, out);
    return retval;
}

// Clean up at end of job.
// Container is assumed to be stopped.
// Copy log output to stderr.
// Remove container and image
//
void cleanup() {
    char cmd[1024];
    vector<string> out;

    snprintf(cmd, sizeof(cmd), "logs %s", container_name);
    docker_conn.command(cmd, out);
    fprintf(stderr, "stderr from container:\n");
    for (string line: out) {
        fprintf(stderr, "%s", line.c_str());
    }
    fprintf(stderr, "stderr end\n");

    snprintf(cmd, sizeof(cmd), "container rm %s", container_name);
    docker_conn.command(cmd, out);

    // don't remove image if it was specified in config
    //
    if (config.image_name.empty()) {
        snprintf(cmd, sizeof(cmd), "image rm %s", image_name);
        docker_conn.command(cmd, out);
    }
}

// check for commands from the client
//
void poll_client_msgs() {
    BOINC_STATUS status;
    boinc_get_status(&status);
#if 0
    fprintf(stderr, "client messages: nohb %d quit %d abort %d suspended %d\n",
        status.no_heartbeat,
        status.quit_request,
        status.abort_request,
        status.suspended
    );
#endif
    if (status.no_heartbeat) {
        fprintf(stderr, "no heartbeat from client - pausing and exiting\n");
        container_op("pause");
        exit(0);
    } else if (status.quit_request) {
        fprintf(stderr, "got quit request from client - pausing container\n");
        container_op("pause");
        exit(0);
    } else if (status.abort_request) {
        fprintf(stderr, "got abort request from client\n");
        container_op("kill");
        cleanup();
        exit(0);
    }
    if (status.suspended) {
        if (config.verbose == VERBOSE_ALL) {
            fprintf(stderr, "client: suspended\n");
        }
        if (running) {
            container_op("pause");
            running = false;
        }
    } else {
        if (config.verbose == VERBOSE_ALL) {
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
// (like .03 sec) so do it infrequently (10 sec)
//
JOB_STATUS poll_app() {
    char cmd[1024];
    vector<string> out;
    int retval;

    snprintf(cmd, sizeof(cmd), "ps --all -f \"name=%s\"", container_name);
    retval = docker_conn.command(cmd, out);
    if (retval) return JOB_FAIL;
    for (string line: out) {
        if (strstr(line.c_str(), container_name)) {
            string needle = "Exited (";
            if (strstr(line.c_str(), needle.c_str())) {
                // JOB_SUCCESS means Docker/Podman succeeded.
                // In this case get the exit code of the container payload.
                //
                line = line.substr(line.find(needle) + needle.size());
                line = line.erase(line.find(')'), string::npos);

                container_exit_code = stoi(line);
                return JOB_SUCCESS;
            }
            return JOB_IN_PROGRESS;
        }
    }
    return JOB_FAIL;
}

// Get CPU and mem usage.
// This is also surprisingly slow; do it every 10 sec
//
int get_stats(RSC_USAGE &ru) {
    char cmd[1024];
    vector<string> out;
    int retval;
    size_t n;

#ifdef __APPLE__
    snprintf(cmd, sizeof(cmd),
        "stats --no-stream  --format \"{{.CPUPerc}}\\ {{.MemUsage}}\" %s",
        container_name
    );
#else
    snprintf(cmd, sizeof(cmd),
        "stats --no-stream  --format \"{{.CPUPerc}} {{.MemUsage}}\" %s",
        container_name
    );
#endif
    retval = docker_conn.command(cmd, out);
    if (retval) return -1;
    if (out.empty()) return -1;

    // output is like
    // 0.00% 420KiB / 503.8GiB
    // but this can be preceded by lines with warning messages
    //
    bool found = false;
    double cpu_pct=0, mem=0;
    char mem_unit=0;
    for (const string& line: out) {
        n = sscanf(line.c_str(), "%lf%% %lf%c", &cpu_pct, &mem, &mem_unit);
        if (n == 3) {
            found = true;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "Can't parse stats reply\n");
        return -1;
    }
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

// read fraction done from file if present, else 0
// called every 10 sec
//
double get_fraction_done() {
    FILE *f = fopen(FRACTION_DONE_FILENAME, "r");
    if (!f) return 0;
    double x, y=0;
    if (fscanf(f, "%lf", &x) == 1) {
        y = x;
    }
    fclose(f);
    return y;
}

//////////  CHECKPOINT  ////////////

#ifdef CHECKPOINT_RESTART

// when we checkpoint, write a file with
//      duration of checkpoint operation
//      CPU time at time of checkpoint
//      docker type
//      [WSL distro name]
// ... the latter 2 in case we somehow change WSL distro or docker type

double get_min_checkpoint_time(double last_dur) {
    double d = last_dur*CHECKPOINT_INTERVAL_FACTOR;
    if (d > MIN_CHECKPOINT_INTERVAL) {
        return dtime() + d;
    } else {
        return dtime() + MIN_CHECKPOINT_INTERVAL;
    }
}

bool have_checkpoint(double &dur, double &lct) {
    int dt, nitems;
    FILE *f = fopen(CHECKPOINT_FILENAME, "r");
    if (!f) return false;
#ifdef _WIN32
    char wsl_distro[256];
    int n = fscanf(f, "%lf\n%lf\n%d\n%255s\n", &dur, &lct, &dt, wsl_distro);
    nitems = 4;
#else
    int n = fscanf(f, "%lf\n%lf\n%d", &dur, &lct, &dt);
    nitems = 3;
#endif
    fclose(f);
    bool file_ok = true;
    if (n != nitems) {
        fprintf(stderr, "bad checkpoint file contents\n");
        file_ok = false;
    } else {
        if (dt != docker_type) {
            fprintf(stderr, "checkpoint file has wrong docker type\n");
            file_ok = false;
        }
#ifdef _WIN32
        if (wsl_distro_name != (string)wsl_distro) {
            fprintf(stderr, "checkpoint file has wrong WSL distro\n");
            file_ok = false;
        }
#endif
    }
    if (!file_ok) {
        boinc_delete_file(CHECKPOINT_FILENAME);
        return false;
    }
    return true;
}

void clear_checkpoint() {
    boinc_delete_file(CHECKPOINT_FILENAME);
}

void make_checkpoint() {
    double start = dtime();
    int retval = container_op("container checkpoint --leave-running");
    if (retval) {
        // Checkpoint failed, probably because CRIU missing.
        fprintf(stderr, "container checkpoint failed\n");
        boinc_delete_file(CHECKPOINT_FILENAME);
        checkpoint_failed = true;
        return;
    }
    FILE *f = fopen(CHECKPOINT_FILENAME, "w");
    if (!f) {
        fprintf(stderr, "Can't create checkpoint status file\n");
        checkpoint_failed = true;
        return;
    }
    double dur = dtime() - start;
#ifdef _WIN32
    fprintf(f, "%f\n%f\n%d\n%s\n",
        dur, cpu_time, docker_type, wsl_distro_name.c_str()
    );
#else
    fprintf(f, "%f\n%f\n%d\n", dur, cpu_time, docker_type);
#endif
    fclose(f);
    fprintf(stderr, "Successfully checkpointed; dur %f cpu %f \n",
        dur, cpu_time
    );
    checkpoint_cpu_time = cpu_time;
    min_checkpoint_time = get_min_checkpoint_time(dur);
}
#endif

//////////  INITIALIZATION  ////////////

#ifdef _WIN32
// find a WSL distro with Docker and set up a command link to it
//
int wsl_init() {
    WSL_DISTRO distro, *dp;
    if (boinc_is_standalone()) {
        distro.distro_name = BOINC_WSL_DISTRO_NAME;
        distro.docker_type = PODMAN;
        distro.boinc_buda_runner_version = 4;
        dp = &distro;
    } else {
        dp = aid.host_info.wsl_distros.find_docker();
        if (!dp) {
            fprintf(stderr, "wsl_init(): no usable WSL distro\n");
            return -1;
        }
    }
    fprintf(stderr, "Using WSL distro %s\n", dp->distro_name.c_str());
    wsl_distro_name = dp->distro_name;
    docker_type = dp->docker_type;
    return docker_conn.init(*dp, config.verbose>0);
}
#endif

#ifndef _WIN32

bool pause_and_exit = false;

// if we get killed, pause the container

void signal_handler(int signum) {
    fprintf(stderr, "got signal %d\n", signum);
    pause_and_exit = true;
}

void init_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) perror("sigaction");
    if (sigaction(SIGQUIT, &sa, NULL) == -1) perror("sigaction");
    if (sigaction(SIGTERM, &sa, NULL) == -1) perror("sigaction");
    if (sigaction(SIGTSTP, &sa, NULL) == -1) perror("sigaction");
}
#endif

int main(int argc, char** argv) {
    BOINC_OPTIONS options;
    int retval;
    bool sporadic = false;
    RSC_USAGE ru;

    for (int j=1; j<argc; j++) {
        if (!strcmp(argv[j], "--sporadic")) {
            sporadic = true;
        } else if (!strcmp(argv[j], "--verbose")) {
            config.verbose = VERBOSE_STD;
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

    // don't write to stderr before this; it won't go to stderr.txt

    fprintf(stderr, "docker_wrapper %d starting\n", DOCKERWRAPPER_RELEASE);
    // parse job.toml
    retval = parse_config_file();
    if (retval) {
        fprintf(stderr, "can't parse config file\n");
        exit(1);
    }

    if (boinc_is_standalone()) {
        config.verbose = VERBOSE_STD;
        strcpy(image_name, "boinc");
        strcpy(container_name, "boinc");
        project_dir = "project";
    } else {
        boinc_get_init_data(aid);
        project_dir = strrchr(aid.project_dir, '/')+1;
        get_image_name();
        get_container_name();
    }

    if (config.verbose) {
        config.print();
    }

    if (sporadic) {
        retval = boinc_sporadic_dir(".");
        if (retval) {
            fprintf(stderr, "can't create sporadic files\n");
            cleanup();
            boinc_finish(retval);
        }
    }

    get_escaped_cwd();

#ifdef _WIN32
    retval = wsl_init();
    if (retval) {
        fprintf(stderr, "wsl_init() failed: %d\n", retval);
        cleanup();
        boinc_finish(1);
    }
#else
    if (boinc_is_standalone()) {
        docker_type = PODMAN;
        fprintf(stderr, "Standalone mode; using Podman\n");
    } else {
        if (!strlen(aid.host_info.docker_version)
            || aid.host_info.docker_type == NONE
        ) {
            fprintf(stderr, "Docker type missing from app_init_data.xml\n");
            fprintf(stderr, "Check project plan class configuration\n");
            cleanup();
            boinc_finish(1);
        }
        docker_type = aid.host_info.docker_type;
    }
    docker_conn.init(docker_type, config.verbose>0);
#endif
    fprintf(stderr, "Using %s\n", docker_type_str(docker_type));

    int state;
    retval = get_container_state(state);
    if (retval) {
        fprintf(stderr, "get_container_state() failed: %d\n", retval);
        boinc_finish(1);
    }
    bool need_start = false;
    switch (state) {
    case CONTAINER_ABSENT:
        if (config.verbose) {
            fprintf(stderr, "creating container %s\n", container_name);
        }
        retval = create_container();
        if (retval) {
            fprintf(stderr, "create_container() failed: %d\n", retval);
            cleanup();
            boinc_finish(1);
        }
        need_start = true;
        break;
    case CONTAINER_CREATED:
        fprintf(stderr, "container is created\n");
        need_start = true;
        break;
    case CONTAINER_RUNNING:
        // already running; do nothing
        fprintf(stderr, "container is already running\n");
        break;
    case CONTAINER_PAUSED:
        fprintf(stderr, "container is paused; unpausing\n");
        retval = container_op("unpause");
        if (retval) {
            fprintf(stderr, "unpause failed; killing\n");
            retval = container_op("kill");
            if (retval) {
                fprintf(stderr, "kill also failed - quitting\n");
                cleanup();
                boinc_finish(1);
            }
            need_start = true;
        }
        break;
    case CONTAINER_EXITED:
        // This probably means the host was rebooted.
        // If we have a checkpoint, restore from there.
        // Otherwise start from the beginning.
        //
        fprintf(stderr, "container is exited; restarting\n");
        need_start = true;
#ifdef CHECKPOINT_RESTART
        double dur, lct;
        if (have_checkpoint(dur, lct)) {
            fprintf(stderr, "have checkpoint - restoring\n");
            retval = container_op("container restore");
                // keep the checkpoint in case we need it again
            if (retval) {
                fprintf(stderr, "restore failed; restarting\n");
                clear_checkpoint();
                need_start = true;
            } else {
                fprintf(stderr, "restore successful; dur %lf lct %lf\n",
                    dur, lct
                );
                last_checkpoint_dur = dur;
                cpu_time = checkpoint_cpu_time = lct;
                min_checkpoint_time = get_min_checkpoint_time(dur);
            }
            // the restore consumed the checkpoint; make a new one
            make_checkpoint();
            need_start = false;
        }
#endif
        break;
    case CONTAINER_OTHER:
        fprintf(stderr, "container is in other state; restarting\n");
        container_op("kill");
        need_start = true;
        break;
    default:
        fprintf(stderr, "bad container state %d\n", state);
        cleanup();
        boinc_finish(1);
    }
    if (need_start) {
        if (config.verbose) {
            fprintf(stderr, "starting container\n");
        }
        retval = container_op("start");
        if (retval) {
            fprintf(stderr, "start failed: %d\n", retval);
            container_op("kill");
            cleanup();
            boinc_finish(1);
        }
    }

#ifndef _WIN32
    init_signal_handler();
#endif

    running = true;
    for (int i=0; ; i++) {
        boinc_sleep(POLL_PERIOD);
            // do this before poll to avoid race condition on startup
#ifndef _WIN32
        if (pause_and_exit) {
            if (running) {
                container_op("pause");
            }
            exit(0);
        }
#endif
        poll_client_msgs();
        if (i%STATUS_PERIOD == 0) {
            // do this stuff every 10 sec
            // First, see if app has exited
            //
            switch(poll_app()) {
            case JOB_FAIL:
                cleanup();
                boinc_finish(1);
                break;
            case JOB_SUCCESS:
                cleanup();
                // JOB_SUCCESS means Docker/Podman succeeded.
                // In this case forward the exit code
                // of the container payload.
                //
                boinc_finish(container_exit_code);
                break;
            default:
                break;
            }

            // If not, get its resource usage
            //
            retval = get_stats(ru);
            if (!retval) {
                cpu_time += STATUS_PERIOD*ru.cpu_frac;
                if (config.verbose == VERBOSE_ALL) {
                    fprintf(stderr, "reporting CPU %f WSS %f\n", cpu_time, ru.wss);
                }
                boinc_report_app_status_aux(
                    cpu_time,
                    checkpoint_cpu_time,
                    get_fraction_done(),
                    0,      // other PID
                    0,0,    // bytes send/received
                    ru.wss
                );
            }

#ifdef CHECKPOINT_RESTART
            // see if we should checkpoint
            //
            if (!checkpoint_failed
                && boinc_time_to_checkpoint()
                && dtime() > min_checkpoint_time
            ) {
                make_checkpoint();
            }
#endif
        }
    }
}
