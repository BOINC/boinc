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

// This file is adapted from code originally supplied by Apple Computer, Inc. 
// The Berkeley Open Infrastructure for Network Computing project has modified 
// the original code and made additions as of September 22, 2006.  The original 
// Apple Public Source License statement appears below:

/*
 * Copyright (c) 2002-2004 Apple Computer, Inc.  All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

// app_stats_mac.C
//

// #define _DEBUG 1

// Put a safety limit on recursion
#define MAX_DESCENDANT_LEVEL 4

// Totals for non_BOINC processes are not useful because most OSs don't
// move idle processes out of RAM, so physical memory is always full
#define GET_NON_BOINC_INFO 0

// We don't need swap space info because
// http://developer.apple.com/documentation/Performance/Conceptual/ManagingMemory/Articles/AboutMemory.html says:
//     Unlike most UNIX-based operating systems, Mac OS X does not use a 
//     preallocated swap partition for virtual memory. Instead, it uses all
//     of the available space on the machine’s boot partition.
// However, the associated overhead is not significant if we are examining 
// only BOINC descendant processes.
#define GET_SWAP_SIZE 1

// The overhead for getting CPU times is not significant if we are 
// examining only BOINC descendant processes.
#define GET_CPU_TIMES 1


#include <cerrno>
#include <sys/types.h>
#include <mach/shared_memory_server.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <sys/sysctl.h>

#include "procinfo.h"

using std::vector;

static int get_boinc_proc_info(int my_pid, int boinc_pid);
static int build_proc_list (vector<PROCINFO>& pi, int boinc_pid);
static void output_child_totals(PROCINFO& pinfo);
static boolean_t appstats_task_update(task_t a_task, vector<PROCINFO>& piv);
static void find_all_descendants(vector<PROCINFO>& piv, int pid, int rlvl);
static void add_child_totals(PROCINFO& pi, vector<PROCINFO>& piv, int pid, int rlvl);
//static void add_others(PROCINFO&, std::vector<PROCINFO>&);
static void sig_pipe(int signo);

#ifdef _DEBUG
static void print_procinfo(PROCINFO& pinfo);
static void vm_size_render(unsigned long long a_size);
#endif

// BOINC helper application to get info about each of the BOINC Client's 
// child processes (including all its descendants) and also totals for 
// all other processes. 
// On the Mac, much of this information is accessible only by the super-user, 
// so this helper application must be run setuid root. 

int main(int argc, char** argv) {
    int boinc_pid, my_pid;
    int retval;
    char buf[256];
    
    if (geteuid() != 0)             // This must be run setuid root
        return EACCES;

    my_pid = getpid();
    boinc_pid = getppid();          // Assumes we were called by BOINC client

    if (argc == 2)
        boinc_pid = atoi(argv[1]);  // Pass in any desired valid pid for testing

    if (signal(SIGPIPE, sig_pipe) == SIG_ERR) {
        fprintf(stderr, "signal error");
        return 0;
    }
    
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    
    while (1) {
        if (fgets(buf, sizeof(buf), stdin) == NULL)
            return 0;
        
        if (feof(stdin))
            return 0;

        retval = get_boinc_proc_info(my_pid, boinc_pid);
    }
    
    return 0;
}

static int get_boinc_proc_info(int my_pid, int boinc_pid) {    
    int retval;
    vector<PROCINFO> piv;
    PROCINFO child_total;
    unsigned int i;
    

    retval = build_proc_list(piv, boinc_pid);
    if (retval)
        return retval;

    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (p.parentid == boinc_pid) {
            if (p.id == my_pid)
                continue;
            
            child_total = p;
            p.is_boinc_app = true;
#ifdef _DEBUG
            printf("\n\nSumming info for process %d and its children:\n", child_total.id);
            print_procinfo(child_total);
#endif
            // look for child processes
 	    add_child_totals(child_total, piv, p.id, 0);
#ifdef _DEBUG
            printf("Totals for process %d and its children:\n", child_total.id);
#endif
            output_child_totals(child_total);
        }
    }
    
    memset(&child_total, 0, sizeof(child_total));
#if 0
#ifdef _DEBUG
    printf("\n\nSumming info for all other processes\n");
#endif
    add_others(child_total, piv);
#endif
    output_child_totals(child_total);   // zero pid signals end of data
    
    return 0;
}


static void output_child_totals(PROCINFO& pinfo) {
    printf("%d %d %.0lf %.0lf %lu %lf %lf\n", 
                pinfo.id, pinfo.parentid, pinfo.working_set_size, pinfo.swap_size, 
                pinfo.page_fault_count, pinfo.user_time, pinfo.kernel_time);
//    fflush(stdout);
}

static int build_proc_list (vector<PROCINFO>& pi, int boinc_pid) {
	boolean_t               retval = FALSE;
	kern_return_t           error;
        mach_port_t             appstats_port;
	processor_set_t         *psets, pset;
	task_t                  *tasks;
	unsigned                i, j, pcnt, tcnt;
        PROCINFO                pinfo;
	int                     pid, mib[4];
	struct kinfo_proc	kinfo;
	size_t			kinfosize;
        
	appstats_port = mach_host_self();

        // First, get a list of all tasks / processes

	error = host_processor_sets(appstats_port, &psets, &pcnt);
	if (error != KERN_SUCCESS) {
		fprintf(stderr,
		    "Error in host_processor_sets(): %s",
		    mach_error_string(error));
		retval = TRUE;
		goto RETURN;
	}

	for (i = 0; i < pcnt; i++) {
                if (retval)
                        break;
                
		error = host_processor_set_priv(appstats_port, psets[i], &pset);
		if (error != KERN_SUCCESS) {
			fprintf(stderr, 
			    "Error in host_processor_set_priv(): %s",
			    mach_error_string(error));
			retval = TRUE;
			break;
		}

		error = processor_set_tasks(pset, &tasks, &tcnt);
		if (error != KERN_SUCCESS) {
			fprintf(stderr,
			    "Error in processor_set_tasks(): %s",
			    mach_error_string(error));
			retval = TRUE;
			break;
		}

		for (j = 0; j < tcnt; j++) {
                        if (retval)
                                break;
                        
                        memset(&pinfo, 0, sizeof(PROCINFO));

                        /* Get pid for this task. */
                        error = pid_for_task(tasks[j], &pid);
                        if (error != KERN_SUCCESS) {
                                /* Not a process, or the process is gone. */
                                continue;
                        }
                        
                        // Get parent pid for each process
                        /* Get kinfo structure for this task. */
                        kinfosize = sizeof(struct kinfo_proc);
                        mib[0] = CTL_KERN;
                        mib[1] = KERN_PROC;
                        mib[2] = KERN_PROC_PID;
                        mib[3] = pid;

                        if (sysctl(mib, 4, &kinfo, &kinfosize, NULL, 0) == -1) {
                                fprintf(stderr,
                                    "%s(): Error in sysctl(): %s", __FUNCTION__,
                                    strerror(errno));
                                retval = TRUE;
                                break;
                        }

                        if (kinfo.kp_proc.p_stat == 0) {
                                /* Zombie process. */
                                continue;
                        }

                        pinfo.id = pid;
                        pinfo.parentid = kinfo.kp_eproc.e_ppid;

                        pi.push_back(pinfo);
                }
        }
        
#if ! GET_NON_BOINC_INFO
        // Next, find all BOINC's decendants and mark them for further study
        if (! retval)
                find_all_descendants(pi, boinc_pid, 0);
#endif
        
        // Now get the process information for each descendant
	for (i = 0; i < pcnt; i++) {
		for (j = 0; j < tcnt; j++) {
                        if (! retval)
                            if (appstats_task_update(tasks[j], pi)) {
                                    retval = TRUE;
                                    goto RETURN;
                            }

			/* Delete task port if it isn't our own. */
			if (tasks[j] != mach_task_self()) {
				mach_port_deallocate(mach_task_self(),
				    tasks[j]);
			}
		}

		error = vm_deallocate((vm_map_t)mach_task_self(),
		    (vm_address_t)tasks, tcnt * sizeof(task_t));
		if (error != KERN_SUCCESS) {
                        if (!retval)
                                fprintf(stderr,
                                    "Error in vm_deallocate(): %s",
                                    mach_error_string(error));
			retval = TRUE;
			goto RETURN;
		}
		if ((error = mach_port_deallocate(mach_task_self(),
			 pset)) != KERN_SUCCESS
		    || (error = mach_port_deallocate(mach_task_self(),
			psets[i])) != KERN_SUCCESS) {
                        if (!retval)
                                fprintf(stderr,
                                    "Error in mach_port_deallocate(): %s",
                                    mach_error_string(error));
			retval = TRUE;
			goto RETURN;
		}
	}

	error = vm_deallocate((vm_map_t)mach_task_self(),
	    (vm_address_t)psets, pcnt * sizeof(processor_set_t));
	if (error != KERN_SUCCESS) {
                if (!retval)
                        fprintf(stderr,
                            "Error in vm_deallocate(): %s",
                            mach_error_string(error));
		retval = TRUE;
		goto RETURN;
	}

	RETURN:
	return retval;

}

/* Update statistics for task a_task. */
static boolean_t appstats_task_update(task_t a_task, vector<PROCINFO>& piv)
{
	boolean_t		retval;
	kern_return_t		error;
	mach_msg_type_number_t	count;
	task_basic_info_data_t	ti;
	vm_address_t		address;
	mach_port_t		object_name;
	vm_region_top_info_data_t info;
	vm_size_t		size;
	thread_array_t		thread_table;
	unsigned int		table_size;
	thread_basic_info_t	thi;
	thread_basic_info_data_t thi_data;
	unsigned		i;
	task_events_info_data_t	events;
        vm_size_t               vsize, rsize;
        PROCINFO                *pinfo;
        int                     pid;
        
	/* Get pid for this task. */
	error = pid_for_task(a_task, &pid);
	if (error != KERN_SUCCESS) {
		/* Not a process, or the process is gone. */
		retval = FALSE;
		goto GONE;
	}
        
        for (i=0; i<piv.size(); i++) {
                pinfo = &piv[i];
                if (pinfo->id == pid)
                        break;
        }

        if (pinfo->id != pid) {
		fprintf(stderr, "pid %d missing from list\n", pid);
		retval = FALSE;
		goto RETURN;
	}
        
#if ! GET_NON_BOINC_INFO
        if (!pinfo->is_boinc_app) {
		retval = FALSE;
		goto RETURN;
	}
#endif        
	/*
	 * Get task_info, which is used for memory usage and CPU usage
	 * statistics.
	 */
	count = TASK_BASIC_INFO_COUNT;
	error = task_info(a_task, TASK_BASIC_INFO, (task_info_t)&ti, &count);
	if (error != KERN_SUCCESS) {
		retval = FALSE;
		goto GONE;
	}

	/*
	 * Get memory usage statistics.
	 */

	/*
	 * Set rsize and vsize; they require no calculation.  (Well, actually,
	 * we adjust vsize if traversing memory objects to not include the
	 * globally shared text and data regions).
	 */
         rsize = ti.resident_size;
#if GET_SWAP_SIZE
         vsize = ti.virtual_size;
            /*
             * Iterate through the VM regions of the process and determine
             * the amount of memory of various types it has mapped.
             */
            for (address = 0; ; address += size) {
                    /* Get memory region. */
                    count = VM_REGION_TOP_INFO_COUNT;
                    if (vm_region(a_task, &address, &size,
                        VM_REGION_TOP_INFO, (vm_region_info_t)&info, &count,
                        &object_name) != KERN_SUCCESS) {
                            /* No more memory regions. */
                            break;
                    }

                    if (address >= GLOBAL_SHARED_TEXT_SEGMENT
                        && address < (GLOBAL_SHARED_DATA_SEGMENT
                        + SHARED_DATA_REGION_SIZE)) {
                            /* This region is private shared. */

                            /*
                             * Check if this process has the globally shared
                             * text and data regions mapped in.  If so, adjust
                             * virtual memory size and exit loop.
                             */
                            if (info.share_mode == SM_EMPTY) {
                                    vm_region_basic_info_data_64_t	b_info;

                                    count = VM_REGION_BASIC_INFO_COUNT_64;
                                    if (vm_region_64(a_task, &address,
                                        &size, VM_REGION_BASIC_INFO,
                                        (vm_region_info_t)&b_info, &count,
                                        &object_name) != KERN_SUCCESS) {
                                            break;
                                    }

                                    if (b_info.reserved) {
                                        vsize -= (SHARED_TEXT_REGION_SIZE + SHARED_DATA_REGION_SIZE);
                                        break;
                                    }
                            }
                }
        }
#else
        vsize = 0;
#endif      // GET_SWAP_SIZE
        pinfo->working_set_size = rsize;
	pinfo->swap_size = vsize;

	/*
	 * Get CPU usage statistics.
	 */

        pinfo->user_time = (double)ti.user_time.seconds + (((double)ti.user_time.microseconds)/1000000.);
        pinfo->kernel_time = (double)ti.system_time.seconds + (((double)ti.system_time.microseconds)/1000000.);

	/* Get number of threads. */
	error = task_threads(a_task, &thread_table, &table_size);
	if (error != KERN_SUCCESS) {
		retval = FALSE;
		goto RETURN;
	}

#if GET_CPU_TIMES
	/* Iterate through threads and collect usage stats. */
	thi = &thi_data;
	for (i = 0; i < table_size; i++) {
		count = THREAD_BASIC_INFO_COUNT;
		if (thread_info(thread_table[i], THREAD_BASIC_INFO,
		    (thread_info_t)thi, &count) == KERN_SUCCESS) {
			if ((thi->flags & TH_FLAGS_IDLE) == 0) {
                            pinfo->user_time += (double)thi->user_time.seconds + (((double)thi->user_time.microseconds)/1000000.);
                            pinfo->kernel_time += (double)thi->system_time.seconds + (((double)thi->system_time.microseconds)/1000000.);
			}
		}
		if (a_task != mach_task_self()) {
			if ((error = mach_port_deallocate(mach_task_self(),
			    thread_table[i])) != KERN_SUCCESS) {
				fprintf(stderr, 
				    "Error in mach_port_deallocate(): %s",
				    mach_error_string(error));
				retval = TRUE;
				goto RETURN;
			}
		}
	}
	if ((error = vm_deallocate(mach_task_self(), (vm_offset_t)thread_table,
	    table_size * sizeof(thread_array_t)) != KERN_SUCCESS)) {
		fprintf(stderr,
		    "Error in vm_deallocate(): %s",
		    mach_error_string(error));
		retval = TRUE;
		goto RETURN;
	}
#endif GET_CPU_TIMES

	/*
	 * Get event counters.
	 */

	count = TASK_EVENTS_INFO_COUNT;
	if (task_info(a_task, TASK_EVENTS_INFO,
	    (task_info_t)&events, &count) != KERN_SUCCESS) {
		/* Error. */
		retval = FALSE;
		goto RETURN;
	} else {
            pinfo->page_fault_count = events.pageins;
        }

	retval = FALSE;
	RETURN:
	GONE:
        
	return retval;
}

// Scan the process table marking all the decendants of the parent 
// process. Loop thru entire table as the entries aren't in order.  
// Recurse at most 5 times to get additional child processes. 
//
static void find_all_descendants(vector<PROCINFO>& piv, int pid, int rlvl) {
    unsigned int i;

    if (rlvl > MAX_DESCENDANT_LEVEL) {
        return;
    }
    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (p.parentid == pid) {
            p.is_boinc_app = true;
            // look for child process of this one
            find_all_descendants(piv, p.id, rlvl+1); // recursion - woo hoo!
        }
    }
}

// Scan the process table adding in CPU time and mem usage. Loop
// thru entire table as the entries aren't in order.  Recurse at
// most 4 times to get additional child processes 
//
static void add_child_totals(PROCINFO& pi, vector<PROCINFO>& piv, int pid, int rlvl) {
    unsigned int i;

    if (rlvl > (MAX_DESCENDANT_LEVEL - 1)) {
        return;
    }
    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (p.parentid == pid) {
            pi.kernel_time += p.kernel_time;
            pi.user_time += p.user_time;
            pi.swap_size += p.swap_size;
            pi.working_set_size += p.working_set_size;
            pi.page_fault_count += p.page_fault_count;
            p.is_boinc_app = true;
#ifdef _DEBUG
            print_procinfo(p);
#endif
            // look for child process of this one
            add_child_totals(pi, piv, p.id, rlvl+1); // recursion - woo hoo!
        }
    }
}

#if 0
static void add_others(PROCINFO& pi, vector<PROCINFO>& piv) {
    unsigned int i;

    memset(&pi, 0, sizeof(pi));
    for (i=0; i<piv.size(); i++) {
        PROCINFO& p = piv[i];
        if (!p.is_boinc_app) {
            pi.kernel_time += p.kernel_time;
            pi.user_time += p.user_time;
            pi.swap_size += p.swap_size;
            pi.working_set_size += p.working_set_size;
            pi.page_fault_count += p.page_fault_count;
            p.is_boinc_app = true;
#ifdef _DEBUG
            print_procinfo(p);
#endif
        }
    }
}
#endif

static void sig_pipe(int signo)
{
	exit(1);
}

#ifdef _DEBUG
static void print_procinfo(PROCINFO& pinfo) {
    unsigned long long rsize, vsize;
    
    rsize = (unsigned long long)pinfo.working_set_size;
    vsize = (unsigned long long)pinfo.swap_size;
    printf("pid=%d, ppid=%d, rm=%llu=", pinfo.id, pinfo.parentid, rsize);
    vm_size_render(rsize);
    printf("=, vm=%llu=", vsize);
    vm_size_render(vsize);
    printf(", pageins=%lu, usertime=%lf, systime=%lf\n", pinfo.page_fault_count, pinfo.user_time, pinfo.kernel_time);
}

/*
 * Render a memory size in units of B, K, M, or G, depending on the value.
 *
 * a_size is ULL, since there are places where VM sizes are capable of
 * overflowing 32 bits, particularly when VM stats are multiplied by the
 * pagesize.
 */
static void vm_size_render(unsigned long long a_size)
{
	if (a_size < 1024) {
		/* 1023B. */
		printf("%4lluB", a_size);
	} else if (a_size < (1024ULL * 1024ULL)) {
		/* K. */
		if (a_size < 10ULL * 1024ULL) {
			/* 9.99K */
			printf("%1.2fK",
			    ((double)a_size) / 1024);
		} else if (a_size < 100ULL * 1024ULL) {
			/* 99.9K */
			printf("%2.1fK",
			    ((double)a_size) / 1024);
		} else {
			/* 1023K */
			printf("%4lluK",
			    a_size / 1024ULL);
		}
	} else if (a_size < (1024ULL * 1024ULL * 1024ULL)) {
		/* M. */
		if (a_size < 10ULL * 1024ULL * 1024ULL) {
			/* 9.99M */
			printf("%1.2fM",
			    ((double)a_size) / (1024 * 1024));
		} else if (a_size < 100ULL * 1024ULL * 1024ULL) {
			/* 99.9M */
			printf("%2.1fM",
			    ((double)a_size) / (1024 * 1024));
		} else {
			/* 1023M */
			printf("%4lluM",
			    a_size / (1024ULL * 1024ULL));
		}
	} else if (a_size < (1024ULL * 1024ULL * 1024ULL * 1024ULL)) {
		/* G. */
		if (a_size < 10ULL * 1024ULL * 1024ULL * 1024ULL) {
			/* 9.99G. */
			printf("%1.2fG",
			    ((double)a_size) / (1024 * 1024 * 1024));
		} else if (a_size < 100ULL * 1024ULL * 1024ULL * 1024ULL) {
			/* 99.9G. */
			printf("%2.1fG",
			    ((double)a_size) / (1024 * 1024 * 1024));
		} else {
			/* 1023G */
			printf("%4lluG",
			    a_size / (1024ULL * 1024ULL * 1024ULL));
		}
	} else if (a_size < (1024ULL * 1024ULL * 1024ULL * 1024ULL)) {
		/* T. */
		if (a_size < 10ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL) {
			/* 9.99T. */
			printf("%1.2fT",
				 ((double)a_size) /
				 (1024ULL * 1024ULL * 1024ULL * 1024ULL));
		} else if (a_size < (100ULL * 1024ULL * 1024ULL * 1024ULL
				     * 1024ULL)) {
			/* 99.9T. */
			printf("%2.1fT",
				 ((double)a_size) /
				 (1024ULL * 1024ULL * 1024ULL * 1024ULL));
		} else {
			/* 1023T */
			printf("%4lluT",
				 a_size /
				 (1024ULL * 1024ULL * 1024ULL * 1024ULL));
		}
	} else {
		/* P. */
		if (a_size < (10ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL
			      * 1024ULL)) {
			/* 9.99P. */
			printf("%1.2fP",
				 ((double)a_size) /
				 (1024ULL * 1024ULL * 1024ULL * 1024ULL
				  * 1024ULL));
		} else if (a_size < (100ULL * 1024ULL * 1024ULL * 1024ULL
				     * 1024ULL)) {
			/* 99.9P. */
			printf("%2.1fP",
				 ((double)a_size) /
				 (1024ULL * 1024ULL * 1024ULL * 1024ULL
				  * 1024ULL));
		} else {
			/* 1023P */
			printf("%4lluP",
				 a_size /
				 (1024ULL * 1024ULL * 1024ULL * 1024ULL
				  * 1024ULL));
		}
	}
}
#endif  // _DEBUG
