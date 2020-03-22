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

import lombok.EqualsAndHashCode;
import lombok.experimental.FieldNameConstants;

@EqualsAndHashCode
@FieldNameConstants(onlyExplicitlyIncluded = true)
public class AppVersion implements Parcelable {
    @FieldNameConstants.Include public String app_name;
    @FieldNameConstants.Include public int version_num;
    public String platform;
    public String plan_class;
    public String api_version;
    public double avg_ncpus;
    public double max_ncpus;

    public double gpu_ram;

    App app;
    Project project;

    AppVersion() {
    }

    AppVersion(String app_name) {
        this.app_name = app_name;
    }

    AppVersion(String app_name, int version_num) {
        this(app_name);
        this.version_num = version_num;
    }

    private AppVersion(Parcel in) {
        this(in.readString(), in.readInt());
        platform = in.readString();
        plan_class = in.readString();
        api_version = in.readString();
        avg_ncpus = in.readDouble();
        max_ncpus = in.readDouble();
        gpu_ram = in.readDouble();

        app = (App) in.readValue(App.class.getClassLoader());
        project = (Project) in.readValue(Project.class.getClassLoader());
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(app_name);
        dest.writeInt(version_num);
        dest.writeString(platform);
        dest.writeString(plan_class);
        dest.writeString(api_version);
        dest.writeDouble(avg_ncpus);
        dest.writeDouble(max_ncpus);
        dest.writeDouble(gpu_ram);

        dest.writeValue(app);
        dest.writeValue(project);
    }

    public static final Parcelable.Creator<AppVersion> CREATOR = new Parcelable.Creator<AppVersion>() {
        public AppVersion createFromParcel(Parcel in) {
            return new AppVersion(in);
        }

        public AppVersion[] newArray(int size) {
            return new AppVersion[size];
        }
    };
}
