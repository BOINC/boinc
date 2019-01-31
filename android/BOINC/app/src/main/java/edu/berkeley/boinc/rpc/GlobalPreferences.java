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

import android.os.Parcel;
import android.os.Parcelable;

public class GlobalPreferences implements Parcelable {

    public double battery_charge_min_pct;
    public double battery_max_temperature;
    public boolean run_on_batteries;
    // poorly named; what it really means is:
    // if false, suspend while on batteries
    public boolean run_if_user_active;
    public boolean run_gpu_if_user_active;
    public double idle_time_to_run;
    //public double suspend_if_no_recent_input;
    public double suspend_cpu_usage;
    public boolean leave_apps_in_memory;
    //public boolean confirm_before_connecting;
    //public boolean hangup_if_dialed;
    public boolean dont_verify_images;
    public double work_buf_min_days;
    public double work_buf_additional_days;
    public double max_ncpus_pct;
    //public int max_ncpus;
    public double cpu_scheduling_period_minutes;
    public double disk_interval;
    public double disk_max_used_gb;
    public double disk_max_used_pct;
    public double disk_min_free_gb;
    //public double vm_max_used_frac;
    public double ram_max_used_busy_frac;
    public double ram_max_used_idle_frac;
    public double max_bytes_sec_up;
    public double max_bytes_sec_down;
    public double cpu_usage_limit;
    public double daily_xfer_limit_mb;
    public int daily_xfer_period_days;
    public boolean override_file_present;
    public boolean network_wifi_only;

    public TimePreferences cpu_times = new TimePreferences();
    public TimePreferences net_times = new TimePreferences();

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        // TODO Auto-generated method stub
        dest.writeDouble(battery_charge_min_pct);
        dest.writeDouble(battery_max_temperature);
        dest.writeDouble(idle_time_to_run);
        dest.writeDouble(suspend_cpu_usage);
        dest.writeDouble(work_buf_min_days);
        dest.writeDouble(work_buf_additional_days);
        dest.writeDouble(max_ncpus_pct);
        dest.writeDouble(cpu_scheduling_period_minutes);
        dest.writeDouble(disk_interval);
        dest.writeDouble(disk_max_used_gb);
        dest.writeDouble(disk_max_used_pct);
        dest.writeDouble(disk_min_free_gb);
        dest.writeDouble(ram_max_used_busy_frac);
        dest.writeDouble(ram_max_used_idle_frac);
        dest.writeDouble(max_bytes_sec_up);
        dest.writeDouble(max_bytes_sec_down);
        dest.writeDouble(cpu_usage_limit);
        dest.writeDouble(daily_xfer_limit_mb);
        dest.writeInt(daily_xfer_period_days);

        dest.writeValue(cpu_times);
        dest.writeValue(net_times);

        dest.writeBooleanArray(new boolean[]{
                run_on_batteries,
                run_if_user_active,
                run_gpu_if_user_active,
                leave_apps_in_memory,
                dont_verify_images,
                override_file_present,
                network_wifi_only
        });
    }

    public GlobalPreferences() {
    }

    private GlobalPreferences(Parcel in) {
        battery_charge_min_pct = in.readDouble();
        battery_max_temperature = in.readDouble();
        idle_time_to_run = in.readDouble();
        suspend_cpu_usage = in.readDouble();
        work_buf_min_days = in.readDouble();
        work_buf_additional_days = in.readDouble();
        max_ncpus_pct = in.readDouble();
        cpu_scheduling_period_minutes = in.readDouble();
        disk_interval = in.readDouble();
        disk_max_used_gb = in.readDouble();
        disk_max_used_pct = in.readDouble();
        disk_min_free_gb = in.readDouble();
        ram_max_used_busy_frac = in.readDouble();
        ram_max_used_idle_frac = in.readDouble();
        max_bytes_sec_up = in.readDouble();
        max_bytes_sec_down = in.readDouble();
        cpu_usage_limit = in.readDouble();
        daily_xfer_limit_mb = in.readDouble();
        daily_xfer_period_days = in.readInt();

        cpu_times = (TimePreferences) in.readValue(TimePreferences.class.getClassLoader());
        net_times = (TimePreferences) in.readValue(TimePreferences.class.getClassLoader());

        boolean[] bArray = in.createBooleanArray();
        run_on_batteries = bArray[0];
        run_if_user_active = bArray[1];
        run_gpu_if_user_active = bArray[2];
        leave_apps_in_memory = bArray[3];
        dont_verify_images = bArray[4];
        override_file_present = bArray[5];
        network_wifi_only = bArray[6];
    }

    public static final Parcelable.Creator<GlobalPreferences> CREATOR = new Parcelable.Creator<GlobalPreferences>() {
        public GlobalPreferences createFromParcel(Parcel in) {
            return new GlobalPreferences(in);
        }

        public GlobalPreferences[] newArray(int size) {
            return new GlobalPreferences[size];
        }
    };
}
