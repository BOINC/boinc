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

public class PlatformInfo implements Parcelable {
    public String name;
    public String friendlyName;
    public String planClass;

    public PlatformInfo(String name, String friendlyName, String planClass) {
        this.name = name;
        this.friendlyName = friendlyName;
        this.planClass = planClass;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int arg1) {
        dest.writeString(name);
        dest.writeString(friendlyName);
        dest.writeString(planClass);
    }

    public PlatformInfo() {
    }

    private PlatformInfo(Parcel in) {
        name = in.readString();
        friendlyName = in.readString();
        planClass = in.readString();
    }

    public static final Parcelable.Creator<PlatformInfo> CREATOR = new Parcelable.Creator<PlatformInfo>() {
        public PlatformInfo createFromParcel(Parcel in) {
            return new PlatformInfo(in);
        }

        public PlatformInfo[] newArray(int size) {
            return new PlatformInfo[size];
        }
    };

}
