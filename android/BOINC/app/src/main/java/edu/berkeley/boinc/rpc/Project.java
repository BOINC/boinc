/*
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
 */

package edu.berkeley.boinc.rpc;

import java.util.ArrayList;

import android.os.Parcel;
import android.os.Parcelable;


public class Project implements Parcelable {
    // all attributes are public for simple access
    public String master_url = "";
    public String project_dir = "";
    public float resource_share = 0;
    public String project_name = "";
    public String user_name = "";
    public String team_name = "";
    public String venue = "";
    public int hostid = 0;
    public ArrayList<GuiUrl> gui_urls = new ArrayList<>();
    public double user_total_credit = 0.0;
    public double user_expavg_credit = 0.0;

    /**
     * As reported by server
     */
    public double host_total_credit = 0;
    /**
     * As reported by server
     */
    public double host_expavg_credit = 0;
    public double disk_usage = 0;

    //	/** # of consecutive times we've failed to contact all scheduling servers */
    public int nrpc_failures = 0;
    public int master_fetch_failures = 0;

    /**
     * Earliest time to contact any server
     */
    public double min_rpc_time = 0;
    public double download_backoff = 0;
    public double upload_backoff = 0;
    public double cpu_short_term_debt = 0;
    public double cpu_long_term_debt = 0;
    public double cpu_backoff_time = 0;
    public double cpu_backoff_interval = 0;
    public double cuda_debt = 0;
    public double cuda_short_term_debt = 0;
    public double cuda_backoff_time = 0;
    public double cuda_backoff_interval = 0;
    public double ati_debt = 0;
    public double ati_short_term_debt = 0;
    public double ati_backoff_time = 0;
    public double ati_backoff_interval = 0;
    public double duration_correction_factor = 0;

    /**
     * Need to fetch and parse the master URL
     */
    public boolean master_url_fetch_pending;

    /**
     * Need to contact scheduling server. Encodes the reason for the request.
     */
    public int sched_rpc_pending = 0;
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
    public double project_files_downloaded_time = 0;
    //
    //	/** when the last successful scheduler RPC finished */
    public double last_rpc_time = 0;

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
        return this.user_name.equalsIgnoreCase(proj.user_name);
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(master_url);
        dest.writeString(project_dir);
        dest.writeFloat(resource_share);
        dest.writeString(project_name);
        dest.writeString(user_name);
        dest.writeString(team_name);
        dest.writeString(venue);
        dest.writeInt(hostid);
        dest.writeList(gui_urls);
        dest.writeDouble(user_total_credit);
        dest.writeDouble(user_expavg_credit);

        dest.writeDouble(host_total_credit);
        dest.writeDouble(host_expavg_credit);
        dest.writeDouble(disk_usage);

        dest.writeInt(nrpc_failures);
        dest.writeInt(master_fetch_failures);

        dest.writeDouble(min_rpc_time);
        dest.writeDouble(download_backoff);
        dest.writeDouble(upload_backoff);
        dest.writeDouble(cpu_short_term_debt);
        dest.writeDouble(cpu_backoff_time);
        dest.writeDouble(cpu_backoff_interval);
        dest.writeDouble(cuda_debt);
        dest.writeDouble(cuda_short_term_debt);
        dest.writeDouble(cuda_backoff_time);
        dest.writeDouble(cuda_backoff_interval);
        dest.writeDouble(ati_debt);
        dest.writeDouble(ati_short_term_debt);
        dest.writeDouble(ati_backoff_time);
        dest.writeDouble(ati_backoff_interval);
        dest.writeDouble(duration_correction_factor);

        dest.writeInt(sched_rpc_pending);
        dest.writeDouble(project_files_downloaded_time);
        dest.writeDouble(last_rpc_time);

        dest.writeBooleanArray(new boolean[]{
                master_url_fetch_pending,
                non_cpu_intensive,
                suspended_via_gui,
                dont_request_more_work,
                scheduler_rpc_in_progress,
                attached_via_acct_mgr,
                detach_when_done,
                ended,
                trickle_up_pending,
                no_cpu_pref,
                no_cuda_pref,
                no_ati_pref
        });
    }

    public Project() {
    }

    private Project(Parcel in) {
        master_url = in.readString();
        project_dir = in.readString();
        resource_share = in.readFloat();
        project_name = in.readString();
        user_name = in.readString();
        team_name = in.readString();
        venue = in.readString();
        hostid = in.readInt();
        in.readList(gui_urls, GuiUrl.class.getClassLoader());
        user_total_credit = in.readDouble();
        user_expavg_credit = in.readDouble();

        host_total_credit = in.readDouble();
        host_expavg_credit = in.readDouble();
        disk_usage = in.readDouble();

        nrpc_failures = in.readInt();
        master_fetch_failures = in.readInt();

        min_rpc_time = in.readDouble();
        download_backoff = in.readDouble();
        upload_backoff = in.readDouble();
        cpu_short_term_debt = in.readDouble();
        cpu_backoff_time = in.readDouble();
        cpu_backoff_interval = in.readDouble();
        cuda_debt = in.readDouble();
        cuda_short_term_debt = in.readDouble();
        cuda_backoff_time = in.readDouble();
        cuda_backoff_interval = in.readDouble();
        ati_debt = in.readDouble();
        ati_short_term_debt = in.readDouble();
        ati_backoff_time = in.readDouble();
        ati_backoff_interval = in.readDouble();
        duration_correction_factor = in.readDouble();

        sched_rpc_pending = in.readInt();
        project_files_downloaded_time = in.readDouble();
        last_rpc_time = in.readDouble();

        boolean[] bArray = in.createBooleanArray();

        master_url_fetch_pending = bArray[0];
        non_cpu_intensive = bArray[1];
        suspended_via_gui = bArray[2];
        dont_request_more_work = bArray[3];
        scheduler_rpc_in_progress = bArray[4];
        attached_via_acct_mgr = bArray[5];
        detach_when_done = bArray[6];
        ended = bArray[7];
        trickle_up_pending = bArray[8];
        no_cpu_pref = bArray[9];
        no_cuda_pref = bArray[10];
        no_ati_pref = bArray[11];

    }

    public static final Parcelable.Creator<Project> CREATOR = new Parcelable.Creator<Project>() {
        public Project createFromParcel(Parcel in) {
            return new Project(in);
        }

        public Project[] newArray(int size) {
            return new Project[size];
        }
    };
}
