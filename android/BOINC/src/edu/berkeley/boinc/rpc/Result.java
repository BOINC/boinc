/*******************************************************************************
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2012 University of California
 * 
 * BOINC is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * BOINC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

package edu.berkeley.boinc.rpc;

public class Result {
	public String  name = "";
	public String  wu_name = "";
	public String  project_url = "";
	public int     version_num;
	public String  plan_class;
	public long    report_deadline;
	public long    received_time;
	public boolean ready_to_report;
	public boolean got_server_ack;
	public double  final_cpu_time;
	public double  final_elapsed_time;
	public int     state;
	public int     scheduler_state;
	public int     exit_status;
	public int     signal;
	public String  stderr_out;
	public boolean suspended_via_gui;
	public boolean project_suspended_via_gui;
	public boolean coproc_missing;
	public boolean gpu_mem_wait;

	// the following defined if active
	public boolean active_task;
	public int     active_task_state;
	public int     app_version_num;
	public int     slot = -1;
	public int     pid;
	public double  checkpoint_cpu_time;
	public double  current_cpu_time;
	public float   fraction_done;
	public double  elapsed_time;
	public double  swap_size;
	public double  working_set_size_smoothed;
	/** actually, estimated elapsed time remaining */
	public double  estimated_cpu_time_remaining;
	public boolean supports_graphics;
	public int     graphics_mode_acked;
	public boolean too_large;
	public boolean needs_shmem;
	public boolean edf_scheduled;
	public String  graphics_exec_path;
	/** only present if graphics_exec_path is */
	public String  slot_path;

	public String  resources = null;
	
	public Project project;
	public AppVersion avp;
	public App app;
	public Workunit wup;

}
