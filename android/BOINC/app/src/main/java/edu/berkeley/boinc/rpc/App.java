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

public class App implements Parcelable {
    public String name = "";
    public String user_friendly_name = "";
    public int non_cpu_intensive = 0;
    public Project project;

    public boolean compare(App myapp) {
        //Check if name is the same
        return this.name.equalsIgnoreCase(myapp.name);
    }

    public final String getName() {
        return user_friendly_name.equals("") ? name : user_friendly_name;
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

    public App() {
    }

    private App(Parcel in) {
        name = in.readString();
        user_friendly_name = in.readString();
        non_cpu_intensive = in.readInt();
        project = (Project) in.readValue(Project.class.getClassLoader());

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
