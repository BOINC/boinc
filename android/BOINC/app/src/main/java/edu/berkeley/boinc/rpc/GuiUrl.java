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
import lombok.ToString;
import lombok.experimental.FieldNameConstants;

@EqualsAndHashCode
@FieldNameConstants
@ToString
public class GuiUrl implements Parcelable {
    public String name;
    public String description;
    public String url;

    GuiUrl(String name, String description, String url) {
        this.name = name;
        this.description = description;
        this.url = url;
    }

    GuiUrl() {
        this("", "", "");
    }

    private GuiUrl(Parcel in) {
        this(in.readString(), in.readString(), in.readString());
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int arg1) {
        dest.writeString(name);
        dest.writeString(description);
        dest.writeString(url);
    }

    public static final Parcelable.Creator<GuiUrl> CREATOR = new Parcelable.Creator<GuiUrl>() {
        public GuiUrl createFromParcel(Parcel in) {
            return new GuiUrl(in);
        }

        public GuiUrl[] newArray(int size) {
            return new GuiUrl[size];
        }
    };
}
