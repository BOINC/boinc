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

// Default screensaver.
// Shows the BOINC logo and a rotating display of
// attached projects and jobs in progress

#ifdef _WIN32
#include "boinc_win.h"
#else
#include <math.h>
#endif
#include <string>
#include <vector>
#ifdef __APPLE__
#include "boinc_api.h"
#include <sys/socket.h>
#endif

#include "diagnostics.h"
#include "gutil.h"
#include "boinc_gl.h"
#include "graphics2.h"
#include "txf_util.h"
#include "network.h"
#include "gui_rpc_client.h"
#include "util.h"
#include "app_ipc.h"
#include "error_numbers.h"

using std::string;
using std::vector;

// text sizes - larger is smaller (???)
#define TASK_INTRO_SIZE     1000.
#define TASK_NONE_SIZE      500.
#define TASK_NONE_REASON_SIZE    800.
#define TASK_PROJ_SIZE      1000.
#define TASK_INFO_SIZE      1200.
#define PROJ_INTRO_SIZE     800.
#define PROJ_NAME_SIZE      500.
#define PROJ_INFO_SIZE      800.
#define ALERT_SIZE          800.

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

// Possible values of iBrandId:
#define BOINC_BRAND_ID 0
#define GRIDREPUBLIC_BRAND_ID 1
#define PROGRESSTHRUPROCESSORS_BRAND_ID 2

#ifdef _GRIDREPUBLIC
static long iBrandId = GRIDREPUBLIC_BRAND_ID;
#else
static long iBrandId = BOINC_BRAND_ID;   // Default value for BOINC
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
    url_to_project_dir((char*)p->master_url.c_str(), dir);
    sprintf(buf, "%s/stat_icon", dir);
}

void slideshow(PROJECT* p) {
    char dir[256], buf[256];
    int i;

    url_to_project_dir((char*)p->master_url.c_str(), dir);
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
    url_to_project_dir((char*)p->master_url.c_str(), dir);
    sprintf(path, "%s/stat_icon", dir);
    boinc_resolve_filename(path, filename, 256);
    pim.icon.load_image_file(filename);
    for (i=0; i<cc_state.apps.size(); i++) {
        APP& app = *cc_state.apps[i];
        if (app.project != p) continue;
        APP_SLIDES as(app.name);
        for (int j=0; j<99; j++) {
            sprintf(path, "%s/slideshow_%s_%02d", dir, app.name.c_str(), j);
            boinc_resolve_filename(path, filename, 256);
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
        float size[3] = {.6, .4, 0};
        logo.draw(pos, size, ALIGN_CENTER, ALIGN_CENTER, alpha);
    }
}

void show_result(RESULT* r, float x, float& y, float alpha) {
    PROGRESS_2D progress;
    char buf[256];
    txf_render_string(.1, x, y, 0, TASK_PROJ_SIZE, white, 0, (char*)r->project->project_name.c_str());
    y -= .02;
    float prog_pos[] = {x, y, 0};
    float prog_c[] = {.5, .4, .1, alpha/2};
    float prog_ci[] = {.1, .8, .2, alpha};
    progress.init(prog_pos, .4, -.01, -0.008, prog_c, prog_ci);
    progress.draw(r->fraction_done);
    sprintf(buf, "%.2f%% ", r->fraction_done*100);
    txf_render_string(.1, x+.41, y, 0, TASK_INFO_SIZE, white, 0, buf);
    y -= .03;
    x += .05;
    sprintf(buf, "Elapsed: %.0f sec  Remaining: %.0f sec", r->elapsed_time, r->estimated_cpu_time_remaining);
    txf_render_string(.1, x, y, 0, TASK_INFO_SIZE, white, 0, buf);
    y -= .03;
    sprintf(buf, "App: %s  Task: %s", r->app->user_friendly_name, r->wup->name);
    txf_render_string(.1, x, y, 0, TASK_INFO_SIZE, white, 0, buf);
    y -= .03;
}

#if 0
void show_coords() {
    int i;
    char buf[256];
    for (i=-100; i< 101; i+=5) {
        sprintf(buf, "%d", i);
        float x = (float)i/100;
        txf_render_string(.1, x, 0, 0, 1000., white, 0, buf);
    }
    for (i=-100; i< 101; i+=5) {
        sprintf(buf, "%d", i);
        float y = (float)i/100;
        txf_render_string(.1, 0, y, 0, 1000., white, 0, buf);
    }
}
#endif

void show_project(unsigned int index, float alpha) {
    float x=.2, y=.6;
    char buf[1024];
    txf_render_string(.1, x, y, 0, PROJ_INTRO_SIZE, white, 0, "This computer is participating in");
    y -= .07;
    PROJECT *p = cc_state.projects[index];
    txf_render_string(.1, x, y, 0, PROJ_NAME_SIZE, white, 0, (char*)p->project_name.c_str());
    y -= .07;
    txf_render_string(.1, x, y, 0, PROJ_INFO_SIZE, white, 0, p->master_url);
    y -= .05;
    sprintf(buf, "User: %s", p->user_name.c_str());
    txf_render_string(.1, x, y, 0, PROJ_INFO_SIZE, white, 0, buf);
    y -= .05;
    if (p->team_name.size()) {
        sprintf(buf, "Team: %s",  p->team_name.c_str());
        txf_render_string(.1, x, y, 0, PROJ_INFO_SIZE, white, 0, buf);
        y -= .05;
    }
    sprintf(buf, "Total credit: %.0f   Average credit: %.0f", p->user_total_credit, p->user_expavg_credit);
    txf_render_string(.1, x, y, 0, PROJ_INFO_SIZE, white, 0, buf);
    y -= .05;
    if (p->suspended_via_gui) {
        txf_render_string(.1, x, y, 0, PROJ_INFO_SIZE, white, 0, "Suspended");
    }
}

void show_disconnected() {
    float x=.3, y=.3;
    if (iBrandId == GRIDREPUBLIC_BRAND_ID) {
        txf_render_string(.1, x, y, 0, ALERT_SIZE, white, 0, "GridRepublic is not running.");
    } else {    
        txf_render_string(.1, x, y, 0, ALERT_SIZE, white, 0, "BOINC is not running.");
    }
}

void show_no_projects() {
    float x=.2, y=.3;
    if (iBrandId == GRIDREPUBLIC_BRAND_ID) {
        txf_render_string(.1, x, y, 0, ALERT_SIZE, white, 0, "GridRepublic is not attached to any projects.");
    } else {    
        txf_render_string(.1, x, y, 0, ALERT_SIZE, white, 0, "BOINC is not attached to any projects.");
    }
    y = .25;
    if (iBrandId == GRIDREPUBLIC_BRAND_ID) {
        txf_render_string(.1, x, y, 0, ALERT_SIZE, white, 0, "Attach to projects using the GridRepublic Desktop.");
    } else {    
        txf_render_string(.1, x, y, 0, ALERT_SIZE, white, 0, "Attach to projects using the BOINC Manager.");
    }
}

#define MAX_JOBS_DISPLAY   4

// index is where to start looking in job array
//
void show_jobs(unsigned int index, double alpha) {
    float x=.1, y=.7;
    unsigned int nfound = 0;
    unsigned int i;
    cc_status.task_suspend_reason &= ~SUSPEND_REASON_CPU_THROTTLE;
    
    if (!cc_status.task_suspend_reason) {
        for (i=0; i<cc_state.results.size(); i++) {
            int j = (i + index) % cc_state.results.size();
            RESULT* r = cc_state.results[j];
            if (!r->active_task) continue;
            if (r->scheduler_state != CPU_SCHED_SCHEDULED) continue;
            if (!nfound) {
                txf_render_string(.1, x, y, 0, TASK_INTRO_SIZE, white, 0, "Running tasks:");
                y -= .05;
            }
            show_result(r, x, y, alpha);
            y -= .05;
            nfound++;
            if (nfound == MAX_JOBS_DISPLAY) break;
        }
    }
    if (!nfound) {
        y = .5;
        txf_render_string(.1, x, y, 0, TASK_NONE_SIZE, white, 0, "No running tasks");
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
            if (iBrandId == GRIDREPUBLIC_BRAND_ID) {
                p = "Computing suspended while GridRepublic is starting up"; break;
            } else {    
                p = "Computing suspended while BOINC is starting up"; break;
            }
        case SUSPEND_REASON_EXCLUSIVE_APP_RUNNING:
            p = "Computing suspended while exclusive application running"; break;
        case SUSPEND_REASON_CPU_USAGE:
            p = "Computing suspended because processor usage is high"; break;
        }
        if (p) {
            y -= .1;
            txf_render_string(.1, x, y, 0, TASK_NONE_REASON_SIZE, white, 0, p);
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

void app_graphics_render(int xs, int ys, double t) {
    double alpha;
    static bool showing_project = false;
    static unsigned int project_index = 0, job_index=0;
    static float logo_pos[3] = {.2, .2, 0};
    int retval;

    if (!connected) {
        if (t > next_connect_time) {
            retval = rpc.init("localhost");
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
                job_index += MAX_JOBS_DISPLAY;
                job_index %= cc_state.results.size();
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

void boinc_app_mouse_move(int x, int y, int left, int middle, int right) {}
void boinc_app_mouse_button(int x, int y, int which, int is_down) {}
void boinc_app_key_press(int, int){}
void boinc_app_key_release(int, int){}

void app_graphics_init() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    txf_load_fonts(".");
#ifdef _WCG
    logo.load_image_file("wcg.bmp");
#else
    if (iBrandId == GRIDREPUBLIC_BRAND_ID) {
        logo.load_image_file("gridrepublic_ss_logo.jpg");
    } else {    
        logo.load_image_file("boinc_logo_black.jpg");
    }
#endif
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
#ifdef _WIN32
    WinsockInitialize();
#endif

    if (test) {
        retval = rpc.init("localhost");
        if (!retval) {
            retval = update_data();
        }
        exit(ERR_CONNECT);
    }
    
#ifdef __APPLE__
    // For GridRepublic, the installer put a branding file in our data directory
    FILE *f = fopen("/Library/Application Support/BOINC Data/Branding", "r");
    if (f) {
        fscanf(f, "BrandId=%ld\n", &iBrandId);
        fclose(f);
    }
#else
#endif

    boinc_graphics_loop(argc, argv, "BOINC screensaver");
    boinc_finish_diag();
#ifdef _WIN32
    WinsockCleanup();
#endif
}
