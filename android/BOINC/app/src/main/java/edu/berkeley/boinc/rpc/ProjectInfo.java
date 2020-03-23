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
import android.support.annotation.NonNull;

import org.apache.commons.lang3.StringUtils;

import java.io.Serializable;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

import lombok.EqualsAndHashCode;

// needs to be serializable to be put into Activity start Intent
@EqualsAndHashCode
public class ProjectInfo implements Serializable, Parcelable {
    private static final long serialVersionUID = -5944047529950035455L; // auto generated
    public String name = "";
    public String url = "";
    public String generalArea;
    public String specificArea;
    public String description;
    public String home;
    public List<String> platforms;
    public String imageUrl;
    public String summary;

    public ProjectInfo() {
    }

    private ProjectInfo(Parcel in) {
        name = in.readString();
        url = in.readString();
        generalArea = in.readString();
        specificArea = in.readString();
        description = in.readString();
        home = in.readString();
        platforms = (ArrayList<String>) in.readSerializable();
        imageUrl = in.readString();
        summary = in.readString();
    }

    @NonNull
    @Override
    public String toString() {
        return "ProjectInfo: " + name + " ; " + url + " ; " + generalArea + " ; " + specificArea
               + " ; " + description + " ; " + home + " ; " + StringUtils.join(platforms, '/')
               + " ; " + imageUrl;
    }

    @Override
    public int describeContents() {
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
        dest.writeSerializable(new ArrayList<>(platforms));
        dest.writeString(imageUrl);
        dest.writeString(summary);
    }

    public static final Parcelable.Creator<ProjectInfo> CREATOR =
            new Parcelable.Creator<ProjectInfo>() {
                public ProjectInfo createFromParcel(Parcel in) {
                    return new ProjectInfo(in);
                }

                public ProjectInfo[] newArray(int size) {
                    return new ProjectInfo[size];
                }
            };

    @SuppressWarnings("java:S115")
    public static final class Fields {
        private Fields() {}

        static final String name = "name";
        static final String url = "url";
        static final String generalArea = "general_area";
        static final String specificArea = "specific_area";
        static final String description = "description";
        static final String home = "home";
        static final String platforms = "platforms";
        static final String imageUrl = "image_url";
        static final String summary = "summary";
    }
}
