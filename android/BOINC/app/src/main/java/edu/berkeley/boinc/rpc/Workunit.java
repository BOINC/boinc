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

public class Workunit implements Parcelable {
    public String name = "";
    public String app_name = "";
    public int version_num;
    public double rsc_fpops_est;
    public double rsc_fpops_bound;
    public double rsc_memory_bound;
    public double rsc_disk_bound;

    public Project project;
    public App app;

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(name);
        dest.writeString(app_name);
        dest.writeInt(version_num);
        dest.writeDouble(rsc_fpops_est);
        dest.writeDouble(rsc_fpops_bound);
        dest.writeDouble(rsc_memory_bound);
        dest.writeDouble(rsc_disk_bound);

        dest.writeValue(project);
        dest.writeValue(app);
    }

    public Workunit() {
    }

    private Workunit(Parcel in) {
        name = in.readString();
        app_name = in.readString();
        version_num = in.readInt();
        rsc_fpops_est = in.readDouble();
        rsc_fpops_bound = in.readDouble();
        rsc_memory_bound = in.readDouble();
        rsc_disk_bound = in.readDouble();

        project = (Project) in.readValue(Project.class.getClassLoader());
        app = (App) in.readValue(App.class.getClassLoader());
    }

    public static final Parcelable.Creator<Workunit> CREATOR = new Parcelable.Creator<Workunit>() {
        public Workunit createFromParcel(Parcel in) {
            return new Workunit(in);
        }

        public Workunit[] newArray(int size) {
            return new Workunit[size];
        }
    };
}
