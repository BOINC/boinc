// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// Includes the methods required for managing saved data on the client
// utilized by the client to managed its deletion policy and communicate with
// the user and the server what the status of the data storage is
//
// Uhhhh... what exactly does the above say????
//

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "filesys.h"
#include "client_msgs.h"
#include "client_types.h"
#include "client_state.h"

using std::vector;

#if 0

// This gets called when the client doesn't have enough disk space to continue
// running active tasks.
// Notify user which project is the greatest offender
// of their data share
//
void data_overflow_notify(PROJECT* project) {
    if (project == NULL) {
        msg_printf(NULL, MSG_ERROR,
            "BOINC has run out of disk space.\n"
            "Please change your General Preferences to allocate more space.\n"
        );
    } else {
        msg_printf(project, MSG_ERROR,
            "BOINC has run out of disk space for %s.\n"
            "Please change your General Preferences to allocate more space.\n",
            project->project_name
        );
    }
}

// Polling function called in do_something()
//
// Makes sure the disk bounds have not been drastically violated
// Returns true if something has been violated and tries to correct
// the problem
// TODO: notify the user of corrections made?
//
bool CLIENT_STATE::data_manager_poll() {
    double tdu, adu;

    static int counter=0;

    if (++counter < 10000) {
        return false;
    }
    counter = 0;

    total_disk_usage(tdu);
    allowed_disk_usage(adu);

    // delete files from offenders only
    //
    if (tdu > adu) {
        if (size_overflow || !projects.size()) {
            return false;
        }
        return fix_data_overflow(tdu, adu);
    }

    if (size_overflow) {
        calc_all_proj_size();
        size_overflow = false;
    }

    return false;
}

bool CLIENT_STATE::fix_data_overflow(double tdu, double adu) {
    double space_needed, deleted_space;
    unsigned int i;
    int priority;
    bool deleted;
    bool tentative;
    PROJECT* p;

    // Check if any projects are tentative
    // could have new prefs in RPC
    //
    for (i=0; i<projects.size(); i++) {
        if (projects[i]->tentative) return false;
    }

    // get accurate sizes as of right now
    //
    calc_all_proj_size();

    // delete files from offenders only

    space_needed = tdu - adu;
    get_more_disk_space(NULL, space_needed);
    total_disk_usage(tdu);

    i=0;
    priority = P_LOW;
    deleted = false;
    
    while (tdu > adu) {
        // there is an offender that doesn't want to give up its files
        // try deleting files from other projects first
        //
        if (i >= projects.size()) {
            if (!deleted){
                if (priority >= P_HIGH) {
                    break;
                } else {
                    priority++;
                }
            }
            deleted = false;
            i=0;
        }
        p = projects[i];
        deleted_space = select_delete(p, 1, priority);
        if (deleted_space != 0) deleted = true;
        tdu -= deleted_space;
        i++;
    }

    i=0;
    deleted = false;
    tentative = false;

    while (tdu > adu) {
        // still not enough space, projects give up non-active WUs
        //
        if (i >= projects.size()) {
            if (!deleted) {
                size_overflow = true;
                data_overflow_notify(NULL);
                return true;
            }
            i=0;
            deleted = false;
        }
        p = projects[i];
        deleted_space = delete_results(p, 1);
        if (deleted_space != 0) deleted = true;
        tdu -= deleted_space;
        i++;
    }
    return false;
}

// calculate the size of all the projects
// use after any deletions to ensure accurate number for size
// 
int CLIENT_STATE::calc_all_proj_size() {
    for (unsigned int i=0; i<projects.size(); ++i) {
        calc_proj_size(projects[i]);
    }
    return 0;
}

// recalculates the total size of the project directory 
// and any associated active task directories
//
int CLIENT_STATE::calc_proj_size(PROJECT* p) {
    p->size = 0;
    project_disk_usage(p, p->size);
    compute_share_disk_size(p);
    return 0;
}


// Any space that is unallocated, returns number of bytes
//
int CLIENT_STATE::anything_free(double& size) {
    double total_size = 0;
    double disk_available;

    for (unsigned int i=0; i<projects.size(); ++i) {
        total_size += projects[i]->size;
    }
    allowed_project_disk_usage(disk_available);
    size = disk_available - total_size;
    if (size > 0) {
        return 0;
    } else {
        size = 0;
        return 1;
    }
}

// Tries to get more disk space for a project. Takes some percentage
// of the disk space that is not utilzed by a project and awards the
// space to the project to be used. Returns true if this was successful,
// false if it was not. It will be uncessful if all the projects are at their
// limits and the currect project is trying to exceed theirs.
//
// This function will try its best to allow a project to grow in size
// It will delete files from all projects that have a larger gap than 
// the current project.
//
bool CLIENT_STATE::get_more_disk_space(PROJECT *p, double space_needed) {
    PROJECT * other_p = NULL;
    double total_space = 0;
    double free_space;
    double offend_size;
    int priority = 0;

    // check to see if there is enough extra space floating around
    // such will be the case after calling this after a deletion
    //
    if (!anything_free(free_space)) {
        total_space += free_space;
        if (total_space > space_needed) {
            return true;
        }
    }

    // If the function has not exited with true, this means that all available
    // space is being used by some combination of projects
    // Check if one of the projects is over its resource share and give the space
    // to the requesting project

    reset_checks();
    priority = P_LOW;

    while (total_space < space_needed) {
        other_p = greatest_offender();
        if (other_p == NULL || (other_p == p)) {
            if (priority > P_HIGH) {
                return false;
            } else {
                priority++;
                reset_checks();
                continue;
            }
        }
        other_p->checked = true;
        
        offend_size = offender(other_p);
        if (space_needed - total_space > offend_size) {
            total_space += select_delete(other_p, offend_size, priority);
        } else {
            total_space += select_delete(other_p, (space_needed - total_space), priority);
        }
    }
    return true;
}

// try and delete this many bytes of data from the greatest offender
// return the amount of disk space that was actually freed by the delete
//
double CLIENT_STATE::select_delete(PROJECT* p, double space_to_delete, int priority) {
    double deleted_space = 0;
    double total_space = 0;

    garbage_collect();

    // if not, then it must start selecting and deleting files
    // until enough space is freed up
    while (total_space < space_to_delete) {
        deleted_space = delete_next_file(p, priority);
        if (deleted_space == 0) break;
        total_space += deleted_space;
    }
    // delete all files that you can after reflagging stickys
    garbage_collect(); 
    // calculate the size of the project after the deletion
    calc_proj_size(p);
    return total_space;
}

// Tries to delete everything but files associated with the active task
// If this function exits with 0 and there is still not enough space,
// the active task needs to be suspended and not restarted until there is
// enough space to run that project. 

double CLIENT_STATE::delete_results(PROJECT *p, double space_to_delete) {
    double deleted_space = 0;
    double oldsize = p->size;

    while (deleted_space < space_to_delete) {
        if (delete_inactive_results(p)) {
            break;
        }
        calc_proj_size(p);
        deleted_space += oldsize - p->size;
    }
    return deleted_space;
}

// Returns the next file based on the deletion policy of the project.
// Returns true if a file that can be deleted was found, 
// false otherwise
//
double CLIENT_STATE::delete_next_file(PROJECT* p, int priority) {
    FILE_INFO* retval = NULL;
    double space_freed = 0;

    if (p->deletion_policy_expire) {
        space_freed = delete_expired(p);
    } 
    if (space_freed == 0) {
        retval = get_priority_or_lru(p, priority);
        if (retval != NULL) {
            retval->sticky = false;
            space_freed = retval->nbytes;
        }
    }
    return space_freed;
}

// Returns the file_info from the project that has the lowest priority
//
FILE_INFO* CLIENT_STATE::get_priority_or_lru(PROJECT* p, int priority) {
    FILE_INFO* fip;
    unsigned int i;
    double lowest_p = 0;
    FILE_INFO* lowest = NULL;

    for (i=0; i<file_infos.size(); i++) {
        fip = file_infos[i];
        
        // files that have no wu's or results and are permenant
        if (fip->ref_cnt==0 && fip->project == p 
            && fip->sticky && fip->priority <= priority) {
                if (lowest == NULL) {
                    lowest = fip;
                    lowest_p = fip->priority;
                } else if (fip->priority < lowest_p) {
                    lowest = file_infos[i];
                    lowest_p = lowest->priority;
                } else if (fip->priority == lowest_p) {
                    if (p->deletion_policy_expire && fip->exp_date < lowest->exp_date) {
                        lowest = file_infos[i];
                    } else if (fip->time_last_used < lowest->time_last_used) {
                        lowest = file_infos[i];
                    }
                }
            }
    }
    return lowest;
}

// Deletes all expired file_infos
// Returns the amount of bytes freed
//
double CLIENT_STATE::delete_expired(PROJECT* p) {
    FILE_INFO* fip;
    double time_now = dtime();
    double space_expired = 0;
    unsigned int i;

    for (i=0; i<file_infos.size(); i++) {
        fip = file_infos[i];
        // files that have no wu's or results and are permenant
        if (fip->ref_cnt==0 && fip->project == p && fip->sticky) {
            if (fip->exp_date > time_now) {
                fip->sticky = false;
                space_expired += fip->nbytes;
            }
        }
    }
    return space_expired;
}

// Delete any files that associated with inactive results
// by marking their results to acknowledged
//
int CLIENT_STATE::delete_inactive_results(PROJECT *p) {
    bool deleted = false;
    RESULT* result;
    unsigned int i;
    
    for (i=0; i<results.size(); i++) {
        result = results[i];
        if (!result->is_active && result->state < RESULT_COMPUTE_DONE) {
            result->got_server_ack = true;
            unstick_result_files(result);
            deleted = true;
        }
    }       
    if (deleted) {
        garbage_collect();
        return 0;
    } else {
        return 1;
    }
}

// should be called after forcebly deleting any result
// ensures any files that were supposed to by permanent are 
// deleted as well, as we are already low on disk space
//
int CLIENT_STATE::unstick_result_files(RESULT *rp) {
    WORKUNIT* wup;
    int retval = 1;
    unsigned int i;
    for (i=0; i<rp->output_files.size(); i++) {
        retval = 0;
        rp->output_files[i].file_info->sticky = false;
    }
    wup = rp->wup;
    for (i=0; i<wup->input_files.size(); i++) {
        retval = 0;
        wup->input_files[i].file_info->sticky = false;
    }
    return retval;
}

// returns the number of bytes the greatest offender is over his usual resource share
// the argument is returned with the number of bytes and the offending
// project is returned
//
PROJECT* CLIENT_STATE::greatest_offender() {
    PROJECT* g_offender = NULL;
    PROJECT* current_suspect;
    double max_offense = 0;
    for (unsigned int i=0; i<projects.size(); ++i) {
        if (!projects[i]->checked){
            current_suspect = projects[i];
            if (offender(current_suspect) > max_offense) {
                g_offender = current_suspect;
                max_offense = offender(current_suspect);
            }
        }
    }
    return g_offender;
}
// returns the number of bytes the project is offending by
// will be negative if it is not an offender
//
double CLIENT_STATE::offender(PROJECT* p) {
    if (p->share_size == 0) {
        calc_all_proj_size();
    }
    return (p->size - p->share_size);
}

// Computes the percentage of the actual resource share that
// has been awarded to this project when compared with the totals
// from all other projects
//
double CLIENT_STATE::compute_resource_share(PROJECT *p) {
    double total_resource_share = 0;

    for (unsigned int i=0; i<projects.size(); ++i) {
        total_resource_share += projects[i]->resource_share;
    }
    return p->resource_share/total_resource_share;
}

// Computes the size of the allowed disk share in number of bytes.
// This number may be smaller than the actual disk usage of the project
// since projects are allowed to grow outside of their disk bounds if there 
// is space not utilzed by other projects
//
int CLIENT_STATE::compute_share_disk_size(PROJECT *p) {
    double disk_available;
    allowed_project_disk_usage(disk_available);
    p->share_size = disk_available * compute_resource_share(p);
    return 0;
}

// resets the checked flag for all projects in gstate to false;
//
int CLIENT_STATE::reset_checks() {
    unsigned int i;
    for (i=0; i<projects.size(); ++i) {
        projects[i]->checked = false;
    }
    return 0;
}

int CLIENT_STATE::total_potential_offender(PROJECT* p, double& tps) {
    PROJECT* other_p;
    unsigned int i;

    garbage_collect();
    anything_free(tps);
    
    for (i=0; i<projects.size(); i++) {
        other_p = projects[i];
        if (other_p != p) {
            tps += proj_potentially_free(other_p);
        }
    }
    if (tps > 0) {
        return 0;
    } else {
        tps = 0;
        return 1;
    }
}

int CLIENT_STATE::total_potential_self(PROJECT* p, double& tps) {
    FILE_INFO* fip;
    unsigned int i;
    total_potential_offender(p, tps);
    for (i=0; i<file_infos.size(); i++) {
        fip = file_infos[i];
        if (fip->ref_cnt == 0 && fip->project == p && fip->sticky) {
            if (!p->deletion_policy_expire) {
                tps += fip->nbytes;
            } else if (p->deletion_policy_expire && (fip->exp_date > dtime())) {
                tps += fip->nbytes;
            }
        }
    }
    if (tps > 0) {
        return 0;
    } else {
        tps = 0;
        return 1;
    }
}

double CLIENT_STATE::proj_potentially_free(PROJECT* p) {
    double offend_share = offender(p);
    double tps = 0;
    FILE_INFO* fip;
    unsigned int i;
    if (offend_share <= 0) {
        return 0;
    }
    for (i=0; i<file_infos.size(); i++) {
        if (tps > offend_share) break;
        fip = file_infos[i];
        if (fip->ref_cnt==0 && fip->project == p && fip->sticky) {
            if (!p->deletion_policy_expire) {
                tps += fip->nbytes;
            } else if (p->deletion_policy_expire && (fip->exp_date > dtime())) {
                tps += fip->nbytes;
            }
        }
    }
    return tps;
}

#endif

const char *BOINC_RCSID_c8458fed1c = "$Id$";
