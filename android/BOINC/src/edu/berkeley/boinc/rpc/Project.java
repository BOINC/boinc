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

import java.util.ArrayList;


public class Project {
	// all attributes are public for simple access
	public String  master_url = "";
	public String project_dir = "";
	public float   resource_share = 0;
	public String  project_name = "";
	public String  user_name = "";
	public String  team_name = "";
	public String  venue = "";
	public int     hostid = 0;
	public ArrayList<GuiUrl> gui_urls = new ArrayList<GuiUrl>();
	public double  user_total_credit = 0;
	public double  user_expavg_credit = 0;

	/** As reported by server */
	public double  host_total_credit = 0;
	/** As reported by server */
	public double  host_expavg_credit = 0;
	public double  disk_usage = 0;
	
//	/** # of consecutive times we've failed to contact all scheduling servers */
	public int     nrpc_failures = 0;
	public int     master_fetch_failures = 0;

	/** Earliest time to contact any server */
	public double  min_rpc_time = 0;
	public double  download_backoff = 0;
	public double  upload_backoff = 0;
	public double  cpu_short_term_debt = 0;
	public double  cpu_long_term_debt = 0;
	public double  cpu_backoff_time = 0;
	public double  cpu_backoff_interval = 0;
	public double  cuda_debt = 0;
	public double  cuda_short_term_debt = 0;
	public double  cuda_backoff_time = 0;
	public double  cuda_backoff_interval = 0;
	public double  ati_debt = 0;
	public double  ati_short_term_debt = 0;
	public double  ati_backoff_time = 0;
	public double  ati_backoff_interval = 0;
	public double  duration_correction_factor = 0;

	/** Need to fetch and parse the master URL */
	public boolean master_url_fetch_pending;

	/** Need to contact scheduling server. Encodes the reason for the request. */
	public int     sched_rpc_pending = 0;
	public boolean non_cpu_intensive = false;
	public boolean suspended_via_gui = false;
	public boolean dont_request_more_work = false;
	public boolean scheduler_rpc_in_progress = false;
	public boolean attached_via_acct_mgr = false;
	public boolean detach_when_done = false;
	public boolean ended = false;
	public boolean trickle_up_pending = false;
	
//	/** When the last project file download was finished
//	 *  (i.e. the time when ALL project files were finished downloading). */
	public double  project_files_downloaded_time = 0;
//
//	/** when the last successful scheduler RPC finished */
	public double  last_rpc_time = 0;

	public boolean no_cpu_pref = false;
	public boolean no_cuda_pref = false;
	public boolean no_ati_pref = false;

	public final String getName() {
		return project_name.equals("") ? master_url : project_name;
	}

	public boolean compare(Project proj) {
		//Test that master_url's match...
		if(!this.master_url.equalsIgnoreCase(proj.master_url)) {
			return false;
		}

		//Test if user_name matches
		if(!this.user_name.equalsIgnoreCase(proj.user_name)) {
			return false;
		}
		return true;
	}
}
