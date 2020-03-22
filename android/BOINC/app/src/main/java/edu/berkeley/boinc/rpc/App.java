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

import org.apache.commons.lang3.StringUtils;

import java.util.Objects;

import lombok.experimental.FieldNameConstants;

@FieldNameConstants
public class App implements Parcelable {
    public String name;
    public String user_friendly_name;
    public int non_cpu_intensive;
    @FieldNameConstants.Exclude public Project project;

    App(String name, String user_friendly_name, int non_cpu_intensive) {
        this.name = name;
        this.user_friendly_name = user_friendly_name;
        this.non_cpu_intensive = non_cpu_intensive;
    }

    public App() {
        this("", "", 0);
    }

    App(String name) {
        this(name, "", 0);
    }

    App(String name, String user_friendly_name) {
        this(name, user_friendly_name, 0);
    }

    private App(Parcel in) {
        this(in.readString(), in.readString(), in.readInt());
        project = (Project) in.readValue(Project.class.getClassLoader());
    }

    public final String getName() {
        return StringUtils.isEmpty(user_friendly_name) ? name : user_friendly_name;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (!(o instanceof App)) {
            return false;
        }

        App app = (App) o;
        return StringUtils.equalsIgnoreCase(name, app.name) &&
               StringUtils.equalsIgnoreCase(user_friendly_name, app.user_friendly_name) &&
               non_cpu_intensive == app.non_cpu_intensive && Objects.equals(project, app.project);
    }

    @Override
    public int hashCode() {
        return Objects.hash(name, user_friendly_name, non_cpu_intensive, project);
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(name);
        dest.writeString(user_friendly_name);
        dest.writeInt(non_cpu_intensive);
        dest.writeValue(project);
    }

    public static final Parcelable.Creator<App> CREATOR = new Parcelable.Creator<App>() {
        public App createFromParcel(Parcel in) {
            return new App(in);
        }

        public App[] newArray(int size) {
            return new App[size];
        }
    };
}
