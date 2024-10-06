// docker_wrapper
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
// If the container already exists
//      this is a restart of the job
//      start the container if it's stopped
// else
//      this is the first run of the job
//      if the image doesn't already exist
//          build image with 'docker build'
//      (need a log around the above?)
//      create the container image with -v to mount slot, project dirs
//      copy input files as needed
// loop: handle msgs from client, check for container exit
// on successful exit
//      copy output files as needed

// Names:
// image name: name:tag
//      name: lower case letters, digits, separators (. _ -); max 4096 chars
//      tag: max 128 chars
//      we'll use: boinc_proj_appname_planclass:version
//
// container name:
//      letters, numbers, _
//      max 255 chars
//      we'll use: boinc_proj_resultname

#include <cstdio>
#include <string>
#include <vector>

#include "toml.h"
    // from https://github.com/mayah/tinytoml

#include "util.h"
#include "boinc_api.h"

using std::string;
using std::vector;

#define POLL_PERIOD 1.0
#define CONFIG_FILE_NAME "job.toml"

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
        printf("slot: %s\n", slot_dir_mount.c_str());
        printf("proj: %s\n", project_dir_mount.c_str());
        for (FILE_COPY c:copy_to_container) {
            printf("to src %s dst %s\n", c.src.c_str(), c.dst.c_str());
        }
        for (FILE_COPY c:copy_from_container) {
            printf("from src %s dst %s\n", c.src.c_str(), c.dst.c_str());
        }
    }
};

char image_name[512];
char container_name[512];
APP_INIT_DATA aid;
CONFIG config;
bool running;
bool verbose = true;

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
int parse_config_file(CONFIG& config) {
    int retval;
    std::ifstream ifs(CONFIG_FILE_NAME);
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

//////////  IMAGE  ////////////

void get_image_name() {
    char *p = strchr(aid.project_dir, '/');
    sprintf(image_name, "boinc_%s_%s_%s:%d",
        p+1, aid.app_name, aid.plan_class, aid.app_version
    );
}

int image_exists(bool &exists) {
    char cmd[256];
    vector<string> out;

    sprintf(cmd, "docker images 2>&1");
    int retval = run_command(cmd, out);
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
    sprintf(cmd, "docker build . -t %s 2>&1", image_name);
    int retval = run_command(cmd, out);
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
        if (verbose) printf("building image\n");
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
    sprintf(container_name, "boinc_%s", aid.result_name);
}

int container_exists(bool &exists) {
    char cmd[1024];
    int retval;
    vector<string> out;

    sprintf(cmd, "docker ps --filter \"name=%s\"",
        container_name
    );
    retval = run_command(cmd, out);
    if (retval) return retval;
    for (string line: out) {
        if (strstr(line.c_str(), container_name)) {
            return true;
        }
    }
    return false;
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
        sprintf(slot_cmd, " -v %s/slots/%d:%s",
            aid.boinc_dir, aid.slot, config.slot_dir_mount.c_str()
        );
    }
    if (config.project_dir_mount.empty()) {
        project_cmd[0] = 0;
    } else {
        sprintf(project_cmd, " -v %s:%s",
            aid.project_dir, config.project_dir_mount.c_str()
        );
    }
    sprintf(cmd, "docker create --name %s %s %s %s",
        container_name,
        slot_cmd, project_cmd,
        image_name
    );
    retval = run_command(cmd, out);
    if (retval) return retval;
    if (error_output(out)) return -1;

    // copy files into container
    //
    for (FILE_COPY &c: config.copy_to_container) {
        sprintf(cmd, "docker cp %s %s:%s",
            c.src.c_str(), container_name, c.dst.c_str()
        );
        retval = run_command(cmd, out);
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
        sprintf(cmd, "docker cp %s:%s %s",
            container_name, c.src.c_str(), c.dst.c_str()
        );
        retval = run_command(cmd, out);
        if (retval) return retval;
    }
    return 0;
}

int remove_container() {
    char cmd[1024];
    vector<string> out;
    sprintf(cmd, "docker rm %s", container_name);
    int retval = run_command(cmd, out);
    return retval;
}

//////////  JOB CONTROL  ////////////

int resume() {
    char cmd[1024];
    vector<string> out;
    sprintf(cmd, "docker start %s", container_name);
    int retval = run_command(cmd, out);
    return retval;
}

int suspend() {
    char cmd[1024];
    vector<string> out;
    sprintf(cmd, "docker stop %s", container_name);
    int retval = run_command(cmd, out);
    return retval;
}

void poll_client_msgs() {
    BOINC_STATUS status;
    boinc_get_status(&status);
    if (status.no_heartbeat || status.quit_request || status.abort_request) {
        fprintf(stderr, "got quit/abort from client\n");
        suspend();
        //remove_container();
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

    sprintf(cmd, "docker ps -all -f \"name=%s\"", container_name);
    retval = run_command(cmd, out);
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
        }
    }

    retval = parse_config_file(config);
    if (retval) {
        fprintf(stderr, "can't parse config file\n");
        exit(1);
    }
    if (verbose) config.print();

    memset(&options, 0, sizeof(options));
    options.main_program = true;
    options.check_heartbeat = true;
    options.handle_process_control = true;

    //boinc_init_options(&options);
    boinc_get_init_data(aid);

    if (boinc_is_standalone()) {
        strcpy(image_name, "boinc_standalone");
        strcpy(container_name, "boinc_test");
        aid.slot = 0;
        strcpy(aid.project_dir, ".");
        strcpy(aid.boinc_dir, ".");
    } else {
        get_image_name();
        get_container_name();
    }

    retval = container_exists(exists);
    if (retval) {
        fprintf(stderr, "container_exists() failed: %d\n", retval);
        boinc_finish(1);
    }
    if (!exists) {
        if (verbose) printf("creating container\n");
        retval = create_container();
        if (retval) {
            fprintf(stderr, "create_container() failed: %d\n", retval);
            boinc_finish(1);
        }
    }
    retval = resume();
    if (retval) {
        fprintf(stderr, "resume() failed: %d\n", retval);
        boinc_finish(1);
    }
    running = true;
    while (1) {
        poll_client_msgs();
        switch(poll_app(ru)) {
        case JOB_FAIL:
            //remove_container();
            boinc_finish(1);    // doesn't return
            break;
        case JOB_SUCCESS:
            copy_files_from_container();
            //remove_container();
            boinc_finish(0);
            break;
        }
        boinc_sleep(POLL_PERIOD);
    }
}
