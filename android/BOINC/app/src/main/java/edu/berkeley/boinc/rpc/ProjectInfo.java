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

import java.io.Serializable;
import java.util.ArrayList;

import android.os.Parcel;
import android.os.Parcelable;

// needs to be serializable to be put into Activity start Intent
public class ProjectInfo implements Serializable, Parcelable {
    private static final long serialVersionUID = -5944047529950035455L; // auto generated
    public String name = "";
    public String url = "";
    public String generalArea;
    public String specificArea;
    public String description;
    public String home;
    public ArrayList<String> platforms;
    public String imageUrl;
    public String summary;

    @Override
    public String toString() {
        StringBuilder platformString = new StringBuilder();
        for(String platform : platforms) {
            platformString.append(platform).append("/");
        }
        return "ProjectInfo: " + name + " ; " + url + " ; " + generalArea + " ; " + specificArea + " ; " + description +
               " ; " + home + " ; " + platformString.toString() + " ; " + imageUrl;
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int arg1) {
        dest.writeString(name);
        dest.writeString(url);
        dest.writeString(generalArea);
        dest.writeString(specificArea);
        dest.writeString(description);
        dest.writeString(home);
        //dest.writeList(platforms);
        dest.writeSerializable(platforms);
        dest.writeString(imageUrl);
        dest.writeString(summary);
    }

    public ProjectInfo() {
    }

    private ProjectInfo(Parcel in) {
        name = in.readString();
        url = in.readString();
        generalArea = in.readString();
        specificArea = in.readString();
        description = in.readString();
        home = in.readString();
        //in.readList(platforms, String.class.getClassLoader());
        platforms = (ArrayList<String>) in.readSerializable();
        imageUrl = in.readString();
        summary = in.readString();

    }

    public static final Parcelable.Creator<ProjectInfo> CREATOR = new Parcelable.Creator<ProjectInfo>() {
        public ProjectInfo createFromParcel(Parcel in) {
            return new ProjectInfo(in);
        }

        public ProjectInfo[] newArray(int size) {
            return new ProjectInfo[size];
        }
    };
}
