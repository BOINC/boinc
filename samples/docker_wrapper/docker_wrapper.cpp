// docker_wrapper: runs a BOINC job in a Docker container
//
// runs in a directory (normally slot dir) containing
//
// Dockerfile
// job.toml
//      optional job config file
// files added to contrainer via Dockerfile
// other input files
//
// For now all files must be <copy_file>
//
// Win:
//      There must be a WSL image containing the Docker engine
//      e.g. an Ubuntu image downloaded from the Windows app store.
//      This image can access the host (Win) filesystem.
//      The wrapper runs a pipe-connected shell in WSL
//      (running in the current dir)
//      and sends commands (e.g. docker commands) via the pipe.
//
// Unix:
//      The host must have the Docker engine.
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
//      (need a log around the above?)
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
//      in the universal model, each WU has a different image
//      so we'll use: boinc_<proj>_<wuname>
//
// container name:
//      letters, numbers, _
//      max 255 chars
//      we'll use: boinc_<proj>_<resultname>

// standalone mode:
// image name: boinc_standalone
// container name: boinc_test
// slot dir: .
// project dir: project/

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

enum JOB_STATUS {JOB_IN_PROGRESS, JOB_SUCCESS, JOB_FAIL};

struct RSC_USAGE {
    double cpu_time;
    double wss;
    void clear() {
        cpu_time = 0;
        wss = 0;
    }
};

struct FILE_COPY {
    string src;
    string dst;
};

// parsed version of job.toml
//
struct CONFIG {
    string slot_dir_mount;
        // mount slot dir here
    string project_dir_mount;
        // mount project dir here
    vector<FILE_COPY> copy_to_container;
    vector<FILE_COPY> copy_from_container;
    void print() {
        fprintf(stderr, "Wrapper config file:\n");
        if (!slot_dir_mount.empty()) {
            fprintf(stderr, "   slot dir mounted at: %s\n", slot_dir_mount.c_str());
        }
        if (!project_dir_mount.empty()) {
            fprintf(stderr, "   project dir mounted at: %s\n", project_dir_mount.c_str());
        }
        for (FILE_COPY c:copy_to_container) {
            fprintf(stderr, "   copy to:src %s dst %s\n", c.src.c_str(), c.dst.c_str());
        }
        for (FILE_COPY c:copy_from_container) {
            fprintf(stderr, "   copy from: src %s dst %s\n", c.src.c_str(), c.dst.c_str());
        }
    }
};

char image_name[512];
char container_name[512];
APP_INIT_DATA aid;
CONFIG config;
bool running;
bool verbose = true;
const char* config_file = "job.toml";
const char* dockerfile = "Dockerfile";
#ifdef _WIN32
WSL_CMD ctl_wc;
const char* cli_prog = "podman";
#else
const char* cli_prog = "docker";
#endif

// parse a list of file copy specs
//
int parse_config_copies(const toml::Value *x, vector<FILE_COPY> &copies) {
    const toml::Array& ar = x->as<toml::Array>();
    for (const toml::Value& a : ar) {
        FILE_COPY copy;
        const toml::Value *b = a.find("src");
        if (!b) return -1;
        copy.src = b->as<string>();
        const toml::Value *c = a.find("dst");
        if (!c) return -1;
        copy.dst = c->as<string>();
        copies.push_back(copy);
    }
    return 0;
}

// parse job config file
//
int parse_config_file() {
    int retval;
    std::ifstream ifs(config_file);
    if (ifs.fail()) {
        return -1;
    }
    toml::ParseResult r = toml::parse(ifs);
    if (!r.valid()) {
        fprintf(stderr, "TOML error: %s\n", r.errorReason.c_str());
        return 1;
    }
    const toml::Value &v = r.value;
    const toml::Value *x;
    x = v.find("slot_dir_mount");
    if (x) {
        config.slot_dir_mount = x->as<string>();
    }
    x = v.find("project_dir_mount");
    if (x) {
        config.project_dir_mount = x->as<string>();
    }
    x = v.find("copy_to_container");
    if (x) {
        retval = parse_config_copies(x, config.copy_to_container);
        if (retval) return retval;
    }
    x = v.find("copy_from_container");
    if (x) {
        retval = parse_config_copies(x, config.copy_from_container);
        if (retval) return retval;
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

inline int run_docker_command(char* cmd, vector<string> &out) {
    int retval;
    if (verbose) {
        fprintf(stderr, "running docker command: %s\n", cmd);
    }
#ifdef _WIN32
    // Win: run the command in the WSL container

    char buf[1024];
    string output;

    sprintf(buf, "%s; echo EOM\n", cmd);
    write_to_pipe(ctl_wc.in_write, buf);
    retval = read_from_pipe(
        ctl_wc.out_read, ctl_wc.proc_handle, output, TIMEOUT, "EOM"
    );
    if (retval) return retval;
    out = split(output, '\n');
#else
    retval = run_command(cmd, out);
    if (retval) return retval;
#endif
    if (verbose) {
        fprintf(stderr, "output:\n");
        for (string line: out) {
            fprintf(stderr, "%s", line.c_str());
        }
    }
    return 0;
}

//////////  IMAGE  ////////////

void get_image_name() {
    char *p = strchr(aid.project_dir, '/');
    sprintf(image_name, "boinc_%s_%s",
        p+1, aid.wu_name
    );
}

int image_exists(bool &exists) {
    char cmd[256];
    vector<string> out;

    sprintf(cmd, "%s images", cli_prog);
    int retval = run_docker_command(cmd, out);
    if (retval) return retval;
    for (string line: out) {
        if (line.find(image_name) != string::npos) {
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
    sprintf(cmd, "%s build . -t %s -f %s", cli_prog, image_name, dockerfile);
    int retval = run_docker_command(cmd, out);
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
    char *p = strchr(aid.project_dir, '/');
    sprintf(container_name, "boinc_%s_%s", p, aid.result_name);
}

int container_exists(bool &exists) {
    char cmd[1024];
    int retval;
    vector<string> out;

    sprintf(cmd, "%s ps --filter \"name=%s\"",
        cli_prog, container_name
    );
    retval = run_docker_command(cmd, out);
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

    if (config.slot_dir_mount.empty()) {
        slot_cmd[0] = 0;
    } else {
        sprintf(slot_cmd, " -v .:%s",
            config.slot_dir_mount.c_str()
        );
    }
    if (config.project_dir_mount.empty()) {
        project_cmd[0] = 0;
    } else {
        sprintf(project_cmd, " -v %s:%s",
            aid.project_dir, config.project_dir_mount.c_str()
        );
    }
    sprintf(cmd, "%s create --name %s %s %s %s",
        cli_prog,
        container_name,
        slot_cmd, project_cmd,
        image_name
    );
    retval = run_docker_command(cmd, out);
    if (retval) return retval;
    if (error_output(out)) return -1;

    // copy files into container
    //
    for (FILE_COPY &c: config.copy_to_container) {
        sprintf(cmd, "%s cp %s %s:%s",
            cli_prog,
            c.src.c_str(), container_name, c.dst.c_str()
        );
        retval = run_docker_command(cmd, out);
        if (retval) return retval;
        if (error_output(out)) return -1;
    }
    return 0;
}

int copy_files_from_container() {
    char cmd[1024];
    int retval;
    vector<string> out;

    for (FILE_COPY &c: config.copy_from_container) {
        sprintf(cmd, "%s cp %s:%s %s",
            cli_prog,
            container_name, c.src.c_str(), c.dst.c_str()
        );
        retval = run_docker_command(cmd, out);
        if (retval) return retval;
    }
    return 0;
}

//////////  JOB CONTROL  ////////////

// Clean up at end of job.
// Show log output if verbose;
// remove container and image
//
void cleanup() {
    char cmd[1024];
    vector<string> out;

    if (verbose) {
        sprintf(cmd, "%s logs %s", cli_prog, container_name);
        run_docker_command(cmd, out);
        fprintf(stderr, "container log:\n");
        for (string line: out) {
            fprintf(stderr, "   %s\n", line.c_str());
        }
    }
    return;
    sprintf(cmd, "%s container rm %s", cli_prog, container_name);
    run_docker_command(cmd, out);

    sprintf(cmd, "%s image rm %s", cli_prog, image_name);
    run_docker_command(cmd, out);
}

int resume() {
    char cmd[1024];
    vector<string> out;
    sprintf(cmd, "%s start %s", cli_prog, container_name);
    int retval = run_docker_command(cmd, out);
    return retval;
}

int suspend() {
    char cmd[1024];
    vector<string> out;
    sprintf(cmd, "%s stop %s", cli_prog, container_name);
    int retval = run_docker_command(cmd, out);
    return retval;
}

void poll_client_msgs() {
    BOINC_STATUS status;
    boinc_get_status(&status);
    if (status.no_heartbeat || status.quit_request || status.abort_request) {
        fprintf(stderr, "got quit/abort from client\n");
        suspend();
        exit(0);
    }
    if (status.suspended) {
#if VERBOSE
        fprintf(stderr, "suspended\n");
#endif
        if (running) suspend();
    } else {
#if VERBOSE
        fprintf(stderr, "not suspended\n");
#endif
        if (!running) resume();
    }
}

JOB_STATUS poll_app(RSC_USAGE &ru) {
    char cmd[1024];
    vector<string> out;
    int retval;

    sprintf(cmd, "%s ps --all -f \"name=%s\"", cli_prog, container_name);
    retval = run_docker_command(cmd, out);
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

#ifdef _WIN32
// find a WSL distro with Docker and set up a command link to it
//
int wsl_init() {
    string distro_name;
    if (boinc_is_standalone()) {
        distro_name = "Ubuntu";
    } else {
        WSL_DISTRO* distro = aid.host_info.wsl_distros.find_docker();
        if (!distro) return -1;
        distro_name = distro->distro_name;
    }
    int retval = ctl_wc.setup();
    if (retval) return retval;
    retval = ctl_wc.run_program_in_wsl(distro_name, "", true);
    if (retval) return retval;
    return 0;
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
        }
    }

#ifdef _WIN32
#if 0
    SetCurrentDirectoryA("C:/ProgramData/BOINC/slots/test_docker_copy");
    config_file = "job_copy.toml";
    dockerfile = "Dockerfile_copy";
#endif
#if 1
    SetCurrentDirectoryA("C:/ProgramData/BOINC/slots/test_docker_mount");
    config_file = "job_mount.toml";
    dockerfile = "Dockerfile_mount";
#endif
#endif

    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;
    boinc_init_options(&options);

    if (boinc_is_standalone()) {
        strcpy(image_name, "boinc_standalone");
        strcpy(container_name, "boinc_test");
        strcpy(aid.project_dir, "project");
    } else {
        boinc_get_init_data(aid);
        get_image_name();
        get_container_name();
    }
    retval = parse_config_file();
    if (retval) {
        fprintf(stderr, "can't parse config file\n");
        exit(1);
    }
    if (verbose) config.print();


#ifdef _WIN32
    retval = wsl_init();
    if (retval) {
        fprintf(stderr, "wsl_init() failed: %d\n", retval);
        boinc_finish(1);
    }
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
    if (verbose) fprintf(stderr, "resuming container\n");
    retval = resume();
    if (retval) {
        fprintf(stderr, "resume() failed: %d\n", retval);
        cleanup();
        boinc_finish(1);
    }
    running = true;
    while (1) {
        poll_client_msgs();
        switch(poll_app(ru)) {
        case JOB_FAIL:
            cleanup();
            boinc_finish(1);
            break;
        case JOB_SUCCESS:
            copy_files_from_container();
            cleanup();
            boinc_finish(0);
            break;
        }
        boinc_sleep(POLL_PERIOD);
    }
}
