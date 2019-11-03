/*******************************************************************************
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2019 University of California
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
 ******************************************************************************/
package edu.berkeley.boinc.utils;

import android.os.Parcel;
import android.os.Parcelable;

public class ErrorCodeDescription implements Parcelable {
    public int code;
    public String description;

    public static final Parcelable.Creator<ErrorCodeDescription> CREATOR = new Parcelable.Creator<ErrorCodeDescription>() {
        public ErrorCodeDescription createFromParcel(Parcel in) {
            return new ErrorCodeDescription(in);
        }

        public ErrorCodeDescription[] newArray(int size) {
            return null;
        }
    };

    public ErrorCodeDescription() {
        code = 0;
        description = "";
    }

    public ErrorCodeDescription(Parcel parcel)
    {
        readFromParcel(parcel);
    }

    public ErrorCodeDescription (int _code, String _description) {
        code = _code;
        description = _description;
    }
    public ErrorCodeDescription (int _code) {
        code = _code;
        description = "";
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel parcel, int i) {
        parcel.writeInt(code);
        parcel.writeString(description);
    }

    public void readFromParcel(Parcel in) {
        code = in.readInt();
        description= in.readString();
    }
}
