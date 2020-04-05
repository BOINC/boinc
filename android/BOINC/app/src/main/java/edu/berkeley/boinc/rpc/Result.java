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

import android.os.Build;
import android.os.Parcel;
import android.os.Parcelable;

import lombok.EqualsAndHashCode;
import lombok.ToString;

@EqualsAndHashCode
@ToString
public class Result implements Parcelable {
    public String name = "";
    public String wu_name = "";
    public String project_url = "";
    public int version_num;
    public String plan_class;
    public long report_deadline;
    public long received_time;
    public boolean ready_to_report;
    public boolean got_server_ack;
    public double final_cpu_time;
    public double final_elapsed_time;
    public int state;
    public int scheduler_state;
    public int exit_status;
    public int signal;
    public String stderr_out;
    public boolean suspended_via_gui;
    public boolean project_suspended_via_gui;
    public boolean coproc_missing;
    public boolean gpu_mem_wait;

    // the following defined if active
    public boolean active_task;
    public int active_task_state;
    public int app_version_num;
    public int slot = -1;
    public int pid;
    public double checkpoint_cpu_time;
    public double current_cpu_time;
    public float fraction_done;
    public double elapsed_time;
    public double swap_size;
    public double working_set_size_smoothed;
    /**
     * actually, estimated elapsed time remaining
     */
    public double estimated_cpu_time_remaining;
    public boolean supports_graphics;
    public int graphics_mode_acked;
    public boolean too_large;
    public boolean needs_shmem;
    public boolean edf_scheduled;
    public String graphics_exec_path;
    /**
     * only present if graphics_exec_path is
     */
    public String slot_path;

    public String resources = null;

    public Project project;
    public AppVersion avp;
    public App app;
    public WorkUnit wup;

    public Result() {
    }

    private Result(Parcel in) {
        name = in.readString();
        wu_name = in.readString();
        project_url = in.readString();
        version_num = in.readInt();
        plan_class = in.readString();
        report_deadline = in.readLong();
        received_time = in.readLong();

        final_cpu_time = in.readDouble();
        final_elapsed_time = in.readDouble();
        state = in.readInt();
        scheduler_state = in.readInt();
        exit_status = in.readInt();
        signal = in.readInt();
        stderr_out = in.readString();

        active_task_state = in.readInt();
        app_version_num = in.readInt();
        slot = in.readInt();
        pid = in.readInt();
        checkpoint_cpu_time = in.readDouble();
        current_cpu_time = in.readDouble();
        fraction_done = in.readFloat();
        elapsed_time = in.readDouble();
        swap_size = in.readDouble();
        working_set_size_smoothed = in.readDouble();
        estimated_cpu_time_remaining = in.readDouble();
        graphics_mode_acked = in.readInt();
        graphics_exec_path = in.readString();

        slot_path = in.readString();

        resources = in.readString();

        project = (Project) in.readValue(Project.class.getClassLoader());
        avp = (AppVersion) in.readValue(AppVersion.class.getClassLoader());
        app = (App) in.readValue(App.class.getClassLoader());
        wup = (WorkUnit) in.readValue(WorkUnit.class.getClassLoader());

        if(Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            boolean[] bArray = in.createBooleanArray();
            assert bArray != null;
            ready_to_report = bArray[0];
            got_server_ack = bArray[1];
            suspended_via_gui = bArray[2];
            project_suspended_via_gui = bArray[3];
            coproc_missing = bArray[4];
            gpu_mem_wait = bArray[5];
            active_task = bArray[6];
            supports_graphics = bArray[7];
            too_large = bArray[8];
            needs_shmem = bArray[9];
            edf_scheduled = bArray[10];
        }
        else {
            ready_to_report = in.readBoolean();
            got_server_ack = in.readBoolean();
            suspended_via_gui = in.readBoolean();
            project_suspended_via_gui = in.readBoolean();
            coproc_missing = in.readBoolean();
            gpu_mem_wait = in.readBoolean();
            active_task = in.readBoolean();
            supports_graphics = in.readBoolean();
            too_large = in.readBoolean();
            needs_shmem = in.readBoolean();
            edf_scheduled = in.readBoolean();
        }
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(name);
        dest.writeString(wu_name);
        dest.writeString(project_url);
        dest.writeInt(version_num);
        dest.writeString(plan_class);
        dest.writeLong(report_deadline);
        dest.writeLong(received_time);

        dest.writeDouble(final_cpu_time);
        dest.writeDouble(final_elapsed_time);
        dest.writeInt(state);
        dest.writeInt(scheduler_state);
        dest.writeInt(exit_status);
        dest.writeInt(signal);
        dest.writeString(stderr_out);

        dest.writeInt(active_task_state);
        dest.writeInt(app_version_num);
        dest.writeInt(slot);
        dest.writeInt(pid);
        dest.writeDouble(checkpoint_cpu_time);
        dest.writeDouble(current_cpu_time);
        dest.writeFloat(fraction_done);
        dest.writeDouble(elapsed_time);
        dest.writeDouble(swap_size);
        dest.writeDouble(working_set_size_smoothed);
        dest.writeDouble(estimated_cpu_time_remaining);
        dest.writeInt(graphics_mode_acked);
        dest.writeString(graphics_exec_path);

        dest.writeString(slot_path);

        dest.writeString(resources);

        dest.writeValue(project);
        dest.writeValue(avp);
        dest.writeValue(app);
        dest.writeValue(wup);

        if(Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            dest.writeBooleanArray(new boolean[]{ready_to_report, got_server_ack, suspended_via_gui,
                                                 project_suspended_via_gui, coproc_missing, gpu_mem_wait,
                                                 active_task, supports_graphics, too_large, needs_shmem,
                                                 edf_scheduled});
        }
        else {
            dest.writeBoolean(ready_to_report);
            dest.writeBoolean(got_server_ack);
            dest.writeBoolean(suspended_via_gui);
            dest.writeBoolean(project_suspended_via_gui);
            dest.writeBoolean(coproc_missing);
            dest.writeBoolean(gpu_mem_wait);
            dest.writeBoolean(active_task);
            dest.writeBoolean(supports_graphics);
            dest.writeBoolean(too_large);
            dest.writeBoolean(needs_shmem);
            dest.writeBoolean(edf_scheduled);
        }
    }

    public static final Parcelable.Creator<Result> CREATOR = new Parcelable.Creator<Result>() {
        public Result createFromParcel(Parcel in) {
            return new Result(in);
        }

        public Result[] newArray(int size) {
            return new Result[size];
        }
    };

    public static final class Fields {
        public static final String WU_NAME = "wu_name";
        public static final String VERSION_NUM = "version_num";
        public static final String REPORT_DEADLINE = "report_deadline";
        public static final String RECEIVED_TIME = "received_time";
        public static final String READY_TO_REPORT = "ready_to_report";
        public static final String GOT_SERVER_ACK = "got_server_ack";
        public static final String FINAL_CPU_TIME = "final_cpu_time";
        public static final String FINAL_ELAPSED_TIME = "final_elapsed_time";
        public static final String STATE = "state";
        public static final String SCHEDULER_STATE = "scheduler_state";
        public static final String EXIT_STATUS = "exit_status";
        public static final String SUSPENDED_VIA_GUI = "suspended_via_gui";
        public static final String PROJECT_SUSPENDED_VIA_GUI = "project_suspended_via_gui";
        public static final String ACTIVE_TASK = "active_task";
        public static final String ACTIVE_TASK_STATE = "active_task_state";
        public static final String APP_VERSION_NUM = "app_version_num";
        public static final String SLOT = "slot";
        public static final String PID = "pid";
        public static final String CHECKPOINT_CPU_TIME = "checkpoint_cpu_time";
        public static final String CURRENT_CPU_TIME = "current_cpu_time";
        public static final String FRACTION_DONE = "fraction_done";
        public static final String ELAPSED_TIME = "elapsed_time";
        public static final String SWAP_SIZE = "swap_size";
        public static final String WORKING_SET_SIZE_SMOOTHED = "working_set_size_smoothed";
        public static final String ESTIMATED_CPU_TIME_REMAINING = "estimated_cpu_time_remaining";
        public static final String SUPPORTS_GRAPHICS = "supports_graphics";
        public static final String GRAPHICS_MODE_ACKED = "graphics_mode_acked";
        public static final String TOO_LARGE = "too_large";
        public static final String NEEDS_SHMEM = "needs_shmem";
        public static final String EDF_SCHEDULED = "edf_scheduled";
        public static final String GRAPHICS_EXEC_PATH = "graphics_exec_path";
        public static final String SLOT_PATH = "slot_path";
        public static final String RESOURCES = "resources";
    }
}
