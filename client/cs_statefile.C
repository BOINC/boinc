// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "parse.h"
#include "error_numbers.h"
#include "filesys.h"
#include "file_names.h"
#include "client_msgs.h"
#include "client_state.h"

void CLIENT_STATE::set_client_state_dirty(char* source) {
    log_messages.printf(CLIENT_MSG_LOG::DEBUG_STATE, "set dirty: %s\n", source);
    client_state_dirty = true;
}

// Parse the client_state.xml file
//
int CLIENT_STATE::parse_state_file() {
    char buf[256];
    PROJECT temp_project, *project=NULL;
    int retval=0;
    int failnum;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);
    if (!boinc_file_exists(STATE_FILE_NAME)) {
        scope_messages.printf("CLIENT_STATE::parse_state_file(): No state file; will create one\n");

        // avoid warning messages about version
        //
        old_major_version = BOINC_MAJOR_VERSION;
        old_minor_version = BOINC_MINOR_VERSION;
        return ERR_FOPEN;
    }

    FILE* f = fopen(STATE_FILE_NAME, "r");
    MIOFILE mf;
    mf.init_file(f);
    fgets(buf, 256, f);
    if (!match_tag(buf, "<client_state>")) {
        retval = ERR_XML_PARSE;
        goto done;
    }
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</client_state>")) {
            retval = 0;
            break;
        } else if (match_tag(buf, "<project>")) {
            temp_project.parse_state(mf);
            project = lookup_project(temp_project.master_url);
            if (project) {
                project->copy_state_fields(temp_project);
            } else {
                msg_printf(NULL, MSG_ERROR, "Project %s found in state file but not prefs.\n",
                    temp_project.master_url);
            }
        } else if (match_tag(buf, "<app>")) {
            APP* app = new APP;
            app->parse(mf);
            if (project) {
                retval = link_app(project, app);
                if (!retval) apps.push_back(app);
            } else {
                delete app;
            }
        } else if (match_tag(buf, "<file_info>")) {
            FILE_INFO* fip = new FILE_INFO;
            fip->parse(mf, false);
            if (project) {
                retval = link_file_info(project, fip);
                if (!retval) file_infos.push_back(fip);
                // If the file had a failure before, there's no reason
                // to start another file transfer
                if (fip->had_failure(failnum)) {
                    if (fip->pers_file_xfer) delete fip->pers_file_xfer;
                    fip->pers_file_xfer = NULL;
                }
                // Init PERS_FILE_XFER and push it onto pers_file_xfer stack
                if (fip->pers_file_xfer) {
                    fip->pers_file_xfer->init(fip, fip->upload_when_present);
                    retval = pers_file_xfers->insert( fip->pers_file_xfer );
                }
            } else {
                delete fip;
            }
        } else if (match_tag(buf, "<app_version>")) {
            APP_VERSION* avp = new APP_VERSION;
            avp->parse(mf);
            if (project) {
                retval = link_app_version(project, avp);
                if (!retval) app_versions.push_back(avp);
            } else {
                delete avp;
            }
        } else if (match_tag(buf, "<workunit>")) {
            WORKUNIT* wup = new WORKUNIT;
            wup->parse(mf);
            if (project) {
                retval = link_workunit(project, wup);
                if (!retval) workunits.push_back(wup);
            } else {
                delete wup;
            }
        } else if (match_tag(buf, "<result>")) {
            RESULT* rp = new RESULT;
            rp->parse_state(mf);
            if (project) {
                retval = link_result(project, rp);
                if (!retval) results.push_back(rp);
            } else {
                msg_printf(NULL, MSG_ERROR,
                    "<result> found before any project\n"
                );
                delete rp;
            }
        } else if (match_tag(buf, "<host_info>")) {
            retval = host_info.parse(mf);
            if (retval) goto done;
        } else if (match_tag(buf, "<time_stats>")) {
            retval = time_stats.parse(mf);
            if (retval) goto done;
        } else if (match_tag(buf, "<net_stats>")) {
            retval = net_stats.parse(mf);
            if (retval) goto done;
        } else if (match_tag(buf, "<active_task_set>")) {
            retval = active_tasks.parse(mf);
            if (retval) goto done;
        } else if (match_tag(buf, "<platform_name>")) {
            // should match our current platform name
        } else if (match_tag(buf, "<version>")) {
            // could put logic here to detect incompatible state files
            // after core client update
        } else if (parse_int(buf, "<core_client_major_version>", old_major_version)) {
        } else if (parse_int(buf, "<core_client_minor_version>", old_minor_version)) {
        } else if (parse_int(buf, "<cpu_sched_period>", cpu_sched_period)) {
        } else if (parse_double(buf, "<cpu_sched_work_done_this_period>", cpu_sched_work_done_this_period)) {
        } else if (match_tag(buf, "<proxy_info>")) {
            retval = pi.parse(mf);
            if (retval) goto done;
        // } else if (parse_int(buf, "<user_run_request/>")) {
        } else if (parse_str(buf, "<host_venue>", host_venue, sizeof(host_venue))) {
        } else scope_messages.printf("CLIENT_STATE::parse_state_file: unrecognized: %s\n", buf);
    }
done:
    fclose(f);

    return retval;
}

// read just the venue from the state file.
//
int CLIENT_STATE::parse_venue() {
    char buf[256];

    if (!boinc_file_exists(STATE_FILE_NAME)) return 0;

    FILE* f = fopen(STATE_FILE_NAME, "r");
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "</client_state>")) {
            break;
        }
        if (parse_str(buf, "<host_venue>", host_venue, sizeof(host_venue))) {
            break;
        }
    }

    fclose(f);

    return 0;
}


// Write the client_state.xml file
//
int CLIENT_STATE::write_state_file() {
    FILE* f = boinc_fopen(STATE_FILE_NEXT, "w");

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);
    scope_messages.printf("CLIENT_STATE::write_state_file(): Writing state file\n");
    if (!f) {
        msg_printf(0, MSG_ERROR, "Can't open temp state file: %s\n", STATE_FILE_NEXT);
        return ERR_FOPEN;
    }
    MIOFILE mf;
    mf.init_file(f);
    int retval = write_state(mf);
    fclose(f);
    if (retval) return retval;

    // the following fails if no current file, so don't check
    //
    retval = boinc_rename(STATE_FILE_NAME, STATE_FILE_PREV);

    retval = boinc_rename(STATE_FILE_NEXT, STATE_FILE_NAME);
    scope_messages.printf("CLIENT_STATE::write_state_file(): Done writing state file\n");
    if (retval) return ERR_RENAME;
    return 0;
}

int CLIENT_STATE::write_state(MIOFILE& f) {
    unsigned int i, j;
    int retval;

    f.printf("<client_state>\n");
    retval = host_info.write(f);
    if (retval) return retval;
    retval = time_stats.write(f, false);
    if (retval) return retval;
    retval = net_stats.write(f, false);
    if (retval) return retval;
    for (j=0; j<projects.size(); j++) {
        PROJECT* p = projects[j];
        retval = p->write_state(f);
        if (retval) return retval;
        for (i=0; i<apps.size(); i++) {
            if (apps[i]->project == p) {
                retval = apps[i]->write(f);
                if (retval) return retval;
            }
        }
        for (i=0; i<file_infos.size(); i++) {
            if (file_infos[i]->project == p) {
                retval = file_infos[i]->write(f, false);
                if (retval) return retval;
            }
        }
        for (i=0; i<app_versions.size(); i++) {
            if (app_versions[i]->project == p) app_versions[i]->write(f);
        }
        for (i=0; i<workunits.size(); i++) {
            if (workunits[i]->project == p) workunits[i]->write(f);
        }
        for (i=0; i<results.size(); i++) {
            if (results[i]->project == p) results[i]->write(f, false);
        }
    }
    active_tasks.write(f);
    f.printf(
        "<platform_name>%s</platform_name>\n"
        "<core_client_major_version>%d</core_client_major_version>\n"
        "<core_client_minor_version>%d</core_client_minor_version>\n",
        platform_name,
        core_client_major_version,
        core_client_minor_version
    );

    // save CPU scheduling state
    //
    f.printf(
        "<cpu_sched_period>%d</cpu_sched_period>\n"
        "<cpu_sched_work_done_this_period>%f</cpu_sched_work_done_this_period>\n",
        cpu_sched_period,
        cpu_sched_work_done_this_period
    );

    // save proxy info
    //
    pi.write(f);
    if (strlen(host_venue)) {
        f.printf("<host_venue>%s</host_venue>\n", host_venue);
    }
    f.printf("</client_state>\n");
    return 0;
}

// Write the client_state.xml file if necessary
// TODO: write no more often than X seconds
//
int CLIENT_STATE::write_state_file_if_needed() {
    int retval;
    if (client_state_dirty) {
        client_state_dirty = false;
        retval = write_state_file();
        if (retval) return retval;
    }
    return 0;
}

// look for app_versions.xml file in project dir.
// If find, get app versions from there,
// and use "anonymous platform" mechanism for this project
//
void CLIENT_STATE::check_anonymous() {
	unsigned int i;
	char dir[256], path[256];
	FILE* f;
	int retval;

	for (i=0; i<projects.size(); i++) {
		PROJECT* p = projects[i];
		get_project_dir(p, dir);
		sprintf(path, "%s%s%s", dir, PATH_SEPARATOR, APP_INFO_FILE_NAME);
		f = fopen(path, "r");
		if (!f) continue;
        msg_printf(p, MSG_INFO,
            "Found %s; using anonymous platform\n", APP_INFO_FILE_NAME
        );

		p->anonymous_platform = true;
			// flag as anonymous even if can't parse file
		retval = parse_app_info(p, f);
		fclose(f);
	}
}

int CLIENT_STATE::parse_app_info(PROJECT* p, FILE* in) {
	char buf[256];
    MIOFILE mf;
    mf.init_file(in);

	while (fgets(buf, 256, in)) {
		if (match_tag(buf, "<app_info>")) continue;
		if (match_tag(buf, "</app_info>")) return 0;
		if (match_tag(buf, "<file_info>")) {
			FILE_INFO* fip = new FILE_INFO;
			if (fip->parse(mf, false)) {
				delete fip;
				continue;
			}
			if (link_file_info(p, fip)) {
				delete fip;
				continue;
			}
			fip->status = FILE_PRESENT;
			file_infos.push_back(fip);
			continue;
		}
		if (match_tag(buf, "<app>")) {
			APP* app = new APP;
			if (app->parse(mf)) {
				delete app;
				continue;
			}
			if (lookup_app(p, app->name)) {
				delete app;
				continue;
			}
			link_app(p, app);
			apps.push_back(app);
			continue;
		}
		if (match_tag(buf, "<app_version>")) {
			APP_VERSION* avp = new APP_VERSION;
			if (avp->parse(mf)) {
				delete avp;
				continue;
			}
			if (gstate.link_app_version(p, avp)) {
				delete avp;
				continue;
			}
			link_app_version(p, avp);
			app_versions.push_back(avp);
			continue;
		}
	}
	return ERR_XML_PARSE;
}

int CLIENT_STATE::write_state_gui(MIOFILE& f) {
    unsigned int i, j;
    int retval;

    f.printf("<client_state>\n");
    for (j=0; j<projects.size(); j++) {
        PROJECT* p = projects[j];
        retval = p->write_state(f, true);
        if (retval) return retval;
        for (i=0; i<apps.size(); i++) {
            if (apps[i]->project == p) {
                retval = apps[i]->write(f);
                if (retval) return retval;
            }
        }
        for (i=0; i<app_versions.size(); i++) {
            if (app_versions[i]->project == p) app_versions[i]->write(f);
        }
        for (i=0; i<workunits.size(); i++) {
            if (workunits[i]->project == p) workunits[i]->write(f);
        }
        for (i=0; i<results.size(); i++) {
            if (results[i]->project == p) results[i]->write_gui(f);
        }
    }
    return 0;
}

int CLIENT_STATE::write_tasks_gui(MIOFILE& f) {
    unsigned int i;
    for(i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        rp->write_gui(f);
    }
    return 0;
}

int CLIENT_STATE::write_file_transfers_gui(MIOFILE& f) {
    unsigned int i;
    for (i=0; i<file_infos.size(); i++) {
        FILE_INFO* fip = file_infos[i];
        if (fip->pers_file_xfer) {
            fip->write_gui(f);
        }
    }
    return 0;
}
