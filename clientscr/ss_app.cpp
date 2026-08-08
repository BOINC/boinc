// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2020 University of California
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

// Default screensaver.
// Shows the BOINC logo and a rotating display of
// attached projects and jobs in progress

#ifdef _WIN32
#include "boinc_win.h"
#else
#include <cmath>
#endif
#include <string>
#include <vector>
#ifdef __APPLE__
#include "boinc_api.h"
#include "mac_branding.h"
#include <sys/socket.h>
#endif

#include "boinc_gl.h"

#include "diagnostics.h"
#include "ttfont.h"
#include "gutil.h"
#include "graphics2.h"
#include "network.h"
#include "gui_rpc_client.h"
#include "util.h"
#include "app_ipc.h"
#include "error_numbers.h"

using std::string;
using std::vector;
using TTFont::ttf_render_string;
using TTFont::ttf_load_fonts;

// text sizes - larger is smaller (???)
#define TASK_INTRO_SIZE     1200.
#define TASK_NONE_SIZE      600.
#define TASK_NONE_REASON_SIZE    800.
#define TASK_PROJ_SIZE      1200.
#define TASK_INFO_SIZE      1350.
#define PROJ_INTRO_SIZE     900.
#define PROJ_NAME_SIZE      600.
#define PROJ_INFO_SIZE      900.
#define ALERT_SIZE          900.

float white[4] = {1., 1., 1., 1.};
TEXTURE_DESC logo;
int width, height;      // window dimensions
bool mouse_down = false;
int mouse_x, mouse_y;

RPC_CLIENT rpc;
bool retry_connect = true;
bool connected = false;
double next_connect_time = 0.0;

CC_STATE cc_state;
CC_STATUS cc_status;

#ifdef __APPLE__
char* brand_name = "BOINC";
char* logo_file = "boinc_logo_black.jpg";
# else
// These defines are used only on Windows builds
#if   defined(_GRIDREPUBLIC)
const char* brand_name = "GridRepublic";
const char* logo_file = "gridrepublic_ss_logo.jpg";
#elif defined(_CHARITYENGINE)
const char* brand_name = "Charity Engine";
const char* logo_file = "CE_ss_logo.jpg";
#elif defined(_PROGRESSTHRUPROCESSORS)
const char* brand_name = "Progress Thru Processors";
const char* logo_file = "progress_ss_logo.jpg";
#elif defined(_WCG)
const char* brand_name = "World Community Grid";
const char* logo_file = "wcg_ss_logo.jpg";
#else
const char* brand_name = "BOINC";
const char* logo_file = "boinc_logo_black.jpg";
#endif
#endif

#if 0
struct APP_SLIDES {
    string name;
    int index;
    double switch_time;
    vector<TEXTURE_DESC> slides;
    APP_SLIDES(string n): name(n), index(0), switch_time(0) {}
};

struct PROJECT_IMAGES {
    string url;
    TEXTURE_DESC icon;
    vector<APP_SLIDES> app_slides;
};

vector<PROJECT_IMAGES> project_images;
void icon_path(PROJECT* p, char* buf) {
    char dir[256];
    url_to_project_dir((char*)p->master_url.c_str(), dir, sizeof(dir));
    sprintf(buf, "%s/stat_icon", dir);
}

void slideshow(PROJECT* p) {
    char dir[256], buf[256];
    int i;

    url_to_project_dir((char*)p->master_url.c_str(), dir, sizeof(dir));
    for (i=0; i<99; i++) {
        sprintf(buf, "%s/slideshow_%02d", dir, i);
    }
}

PROJECT_IMAGES* get_project_images(PROJECT* p) {
    unsigned int i;
    char dir[256], path[256], filename[256];

    for (i=0; i<project_images.size(); i++) {
        PROJECT_IMAGES& pi = project_images[i];
        if (pi.url == p->master_url) return &pi;
    }
    PROJECT_IMAGES pim;
    pim.url = p->master_url;
    url_to_project_dir((char*)p->master_url.c_str(), dir, sizeof(dir));
    sprintf(path, "%s/stat_icon", dir);
    resolve_soft_link(path, filename, sizeof(filename));
    pim.icon.load_image_file(filename);
    for (i=0; i<cc_state.apps.size(); i++) {
        APP& app = *cc_state.apps[i];
        if (app.project != p) continue;
        APP_SLIDES as(app.name);
        for (int j=0; j<99; j++) {
            sprintf(path, "%s/slideshow_%s_%02d", dir, app.name.c_str(), j);
            resolve_soft_link(path, filename, sizeof(filename));
            TEXTURE_DESC td;
            int retval = td.load_image_file(filename);
            if (retval) break;
            as.slides.push_back(td);
        }
        pim.app_slides.push_back(as);
    }
    project_images.push_back(pim);
    return &(project_images.back());
}

#endif




// set up lighting model
//
static void init_lights() {
   GLfloat ambient[] = {1., 1., 1., 1.0};
   GLfloat position[] = {-13.0, 6.0, 20.0, 1.0};
   GLfloat dir[] = {-1, -.5, -3, 1.0};
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_POSITION, position);
   glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
}

static void draw_logo(float* pos, float alpha) {
    if (logo.present) {
        float size[3] = {.6f, .4f, 0};
        logo.draw(pos, size, ALIGN_CENTER, ALIGN_CENTER, alpha);
    }
}

void show_result(RESULT* r, float x, float& y, float alpha) {
    PROGRESS_2D progress;
    char buf[256];
    ttf_render_string(x, y, 0, TASK_PROJ_SIZE, white, (char*)r->project->project_name.c_str());
    y -= .02f;
    float prog_pos[] = {x, y, 0};
    float prog_c[] = {.5f, .4f, .1f, alpha/2};
    float prog_ci[] = {.1f, .8f, .2f, alpha};
    progress.init(prog_pos, .4f, -.01f, -0.008f, prog_c, prog_ci);
    progress.draw(r->fraction_done);
    snprintf(buf, sizeof(buf), "%.2f%% ", r->fraction_done*100);
    ttf_render_string(x+.41, y, 0, TASK_INFO_SIZE, white, buf);
    y -= .03f;
    x += .05f;
    snprintf(buf, sizeof(buf), "Elapsed: %.0f sec  Remaining: %.0f sec", r->elapsed_time, r->estimated_cpu_time_remaining);
    ttf_render_string(x, y, 0, TASK_INFO_SIZE, white, buf);
    y -= .03f;
    snprintf(buf, sizeof(buf), "App: %s  Task: %s", r->app->user_friendly_name, r->wup->name);
    ttf_render_string(x, y, 0, TASK_INFO_SIZE, white, buf);
    y -= .03f;
}

#if 0
void show_coords() {
    int i;
    char buf[256];
    for (i=-100; i< 101; i+=5) {
        sprintf(buf, "%d", i);
        float x = (float)i/100;
        ttf_render_string(x, 0, 0, 1000., white, buf);
    }
    for (i=-100; i< 101; i+=5) {
        sprintf(buf, "%d", i);
        float y = (float)i/100;
        ttf_render_string(0, y, 0, 1000., white, buf);
    }
}
#endif

void show_project(unsigned int index, float /*alpha*/) {
    float x=.2f, y=.6f;
    char buf[1024];
    ttf_render_string(x, y, 0, PROJ_INTRO_SIZE, white, "This computer is participating in");
    y -= .07f;
    PROJECT *p = cc_state.projects[index];
    ttf_render_string(x, y, 0, PROJ_NAME_SIZE, white, (char*)p->project_name.c_str());
    y -= .07f;
    ttf_render_string(x, y, 0, PROJ_INFO_SIZE, white, p->master_url);
    y -= .05f;
    snprintf(buf, sizeof(buf), "User: %s", p->user_name.c_str());
    ttf_render_string(x, y, 0, PROJ_INFO_SIZE, white, buf);
    y -= .05f;
    if (p->team_name.size()) {
        snprintf(buf, sizeof(buf), "Team: %s",  p->team_name.c_str());
        ttf_render_string(x, y, 0, PROJ_INFO_SIZE, white, buf);
        y -= .05f;
    }
    snprintf(buf, sizeof(buf), "Total credit: %.0f   Average credit: %.0f", p->user_total_credit, p->user_expavg_credit);
    ttf_render_string(x, y, 0, PROJ_INFO_SIZE, white, buf);
    y -= .05f;
    if (p->suspended_via_gui) {
        ttf_render_string(x, y, 0, PROJ_INFO_SIZE, white, "Suspended");
    }
}

void show_disconnected() {
    float x=.3f, y=.3f;
    char buf[256];
    snprintf(buf, sizeof(buf), "%s is not running.", brand_name);
    ttf_render_string(x, y, 0, ALERT_SIZE, white, buf);
}

void show_no_projects() {
    float x=.2f, y=.3f;
    char buf[256];
    snprintf(buf, sizeof(buf), "%s is not attached to any projects.", brand_name);
    ttf_render_string(x, y, 0, ALERT_SIZE, white, buf);
    y = .25f;
    snprintf(buf, sizeof(buf), "Attach to projects using %s.", brand_name);
    ttf_render_string(x, y, 0, ALERT_SIZE, white, buf);
}

#define MAX_JOBS_DISPLAY   4

// index is where to start looking in job array
//
void show_jobs(unsigned int index, double alpha) {
    float x=.1f, y=.7f;
    unsigned int nfound = 0;
    unsigned int i;
    cc_status.task_suspend_reason &= ~SUSPEND_REASON_CPU_THROTTLE;
    char buf[256];

    if (!cc_status.task_suspend_reason) {
        for (i=0; i<cc_state.results.size(); i++) {
            int j = (i + index) % cc_state.results.size();
            RESULT* r = cc_state.results[j];
            if (!r->active_task) continue;
            if (r->scheduler_state != CPU_SCHED_SCHEDULED) continue;
            if (!nfound) {
                ttf_render_string(x, y, 0, TASK_INTRO_SIZE, white, "Running tasks:");
                y -= .05f;
            }
            show_result(r, x, y, alpha);
            y -= .05f;
            nfound++;
            if (nfound == MAX_JOBS_DISPLAY) break;
        }
    }
    if (!nfound) {
        y = .5f;
        ttf_render_string(x, y, 0, TASK_NONE_SIZE, white, "No running tasks");
        char *p = 0;
        switch (cc_status.task_suspend_reason) {
        case SUSPEND_REASON_BATTERIES:
            p = "Computer is running on batteries"; break;
        case SUSPEND_REASON_USER_ACTIVE:
            p = "Computer is in use"; break;
        case SUSPEND_REASON_USER_REQ:
            p = "Computing suspended by user"; break;
        case SUSPEND_REASON_TIME_OF_DAY:
            p = "Computing suspended during this time of day"; break;
        case SUSPEND_REASON_BENCHMARKS:
            p = "Computing suspended while running benchmarks"; break;
        case SUSPEND_REASON_DISK_SIZE:
            p = "Computing suspended because no disk space"; break;
        case SUSPEND_REASON_NO_RECENT_INPUT:
            p = "Computing suspended while computer not in use"; break;
        case SUSPEND_REASON_INITIAL_DELAY:
            snprintf(buf, sizeof(buf),
                "Computing suspended while %s is starting up", brand_name
            );
            p = buf;
            break;
        case SUSPEND_REASON_EXCLUSIVE_APP_RUNNING:
            p = "Computing suspended while exclusive application running"; break;
        case SUSPEND_REASON_CPU_USAGE:
            p = "Computing suspended because processor usage is high"; break;
        }
        if (p) {
            y -= .1f;
            ttf_render_string(x, y, 0, TASK_NONE_REASON_SIZE, white, p);
        }
    }
}

int update_data() {
    int retval = rpc.get_state(cc_state);
    if (!retval) {
        retval = rpc.get_cc_status(cc_status);
    }
    return retval;
}

struct FADER {
    double grow, on, fade, off;
    double start, total;
    FADER(double g, double n, double f, double o) {
        grow = g;
        on = n;
        fade = f;
        off = o;
        start = 0;
        total = grow + on + fade + off;
    }
    bool value(double t, double& v) {
        if (!start) {
            start = t;
            v = 0;
            return false;
        }
        double dt = t - start;
        if (dt > total) {
            start = t;
            v = 0;
            return true;
        }
        if (dt < grow) {
            v = dt/grow;
        } else if (dt < grow+on) {
            v = 1;
        } else if (dt < grow + on + fade) {
            double x = dt-(grow+on);
            v = 1-(x/fade);
        } else {
            v = 0;
        }
        return false;
    }
};

FADER logo_fader(5,5,5,2);
FADER info_fader(4,4,4,1);

void app_graphics_render(int , int , double t) {
    double alpha;
    static bool showing_project = false;
    static unsigned int project_index = 0, job_index=0;
    static float logo_pos[3] = {.2f, .2f, 0};
    int retval;

    if (!connected) {
        if (t > next_connect_time) {
            retval = rpc.init(NULL);
            if (!retval) {
                retval = update_data();
            }
            if (retval) {
                if (!retry_connect) {
                    exit(ERR_CONNECT);
                }
                next_connect_time = t + 10;
            } else {
                connected = true;
            }
        }
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw logo first - it's in background
    //
    mode_unshaded();
    mode_ortho();
    if (logo_fader.value(t, alpha)) {
        logo_pos[0] = drand()*.4;
        logo_pos[1] = drand()*.4;
    }
    draw_logo(logo_pos, (float)alpha);

    if (info_fader.value(t, alpha)) {
        retval = update_data();
        if (retval) {
            if (!retry_connect) {
                exit(ERR_CONNECT);
            }
            connected = false;
            next_connect_time = t + 10;
        } else {
            if (showing_project) {
                showing_project = false;
                project_index++;
            } else {
                int n = (int)cc_state.results.size();
                if (n) {
                    job_index += MAX_JOBS_DISPLAY;
                    job_index %= n;
                } else {
                    job_index = 0;
                }
                showing_project = true;
            }
        }
    }
    white[3] = alpha;
    if (connected) {
        if (cc_state.projects.size() == 0) {
            show_no_projects();
        } else if (showing_project) {
            if (project_index >= cc_state.projects.size()) {
                project_index = 0;
            }
            show_project(project_index, alpha);
        } else {
            show_jobs(job_index, alpha);
        }
    } else {
        show_disconnected();
    }
    ortho_done();
}

void app_graphics_resize(int w, int h){
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}

void boinc_app_mouse_move(int , int , int , int , int ) {}
void boinc_app_mouse_button(int , int , int , int ) {}
void boinc_app_key_press(int, int){}
void boinc_app_key_release(int, int){}

void app_graphics_init() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    ttf_load_fonts(".");
    logo.load_image_file(logo_file);
    init_lights();
}

int main(int argc, char** argv) {
    int retval;
    bool test = false;

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--test")) {
            test = true;
        }
        if (!strcmp(argv[i], "--retry_connect")) {
            retry_connect = true;
        }
    }

    // Initialize the BOINC Diagnostics Framework
    int dwDiagnosticsFlags =
#ifdef _DEBUG
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_MEMORYLEAKCHECKENABLED |
#endif
        BOINC_DIAG_DUMPCALLSTACKENABLED |
#ifndef __APPLE__   // Can't access user's directories under sandbox security
        BOINC_DIAG_PERUSERLOGFILES |
#endif
        BOINC_DIAG_REDIRECTSTDERR |
        BOINC_DIAG_REDIRECTSTDOUT |
        BOINC_DIAG_TRACETOSTDOUT;

    diagnostics_init(dwDiagnosticsFlags, "stdoutscrgfx", "stderrscrgfx");

#ifdef _WIN32
    WinsockInitialize();
#endif

    if (test) {
        retval = rpc.init(NULL);
        if (!retval) {
            retval = update_data();
        }
        exit(ERR_CONNECT);
    }

#ifdef __APPLE__
    long brandId = 0;
    // For branded installs, the installer put a branding file in our data directory
    FILE *f = fopen("/Library/Application Support/BOINC Data/Branding", "r");
    if (f) {
        fscanf(f, "BrandId=%ld\n", &brandId);
        fclose(f);
    }
    if ((brandId < 0) || (brandId > (NUMBRANDS-1))) {
        brandId = 0;
    }
    brand_name = brandName[brandId];
    logo_file = logoFile[brandId];
#endif

    boinc_graphics_loop(argc, argv, "BOINC screensaver");

    boinc_finish_diag();

#ifdef _WIN32
    WinsockCleanup();
#endif

    return 0;
}
