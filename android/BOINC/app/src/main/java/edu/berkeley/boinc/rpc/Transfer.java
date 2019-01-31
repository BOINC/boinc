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

import android.os.Parcel;
import android.os.Parcelable;

public class Transfer implements Serializable, Parcelable {
    private static final long serialVersionUID = 1L;
    public String name;
    public String project_url;
    public boolean generated_locally;
    public long nbytes;
    public boolean xfer_active;
    public boolean is_upload;
    public int status;
    public long next_request_time;
    public long time_so_far;
    public long bytes_xferred;
    public float xfer_speed;
    public long project_backoff;

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        // TODO Auto-generated method stub
        dest.writeString(name);
        dest.writeString(project_url);
        dest.writeLong(nbytes);
        dest.writeInt(status);
        dest.writeLong(next_request_time);
        dest.writeLong(time_so_far);
        dest.writeLong(bytes_xferred);
        dest.writeFloat(xfer_speed);
        dest.writeLong(project_backoff);

        dest.writeBooleanArray(new boolean[]{
                generated_locally,
                xfer_active,
                is_upload
        });
    }

    public Transfer() {
    }

    private Transfer(Parcel in) {
        name = in.readString();
        project_url = in.readString();
        nbytes = in.readLong();
        status = in.readInt();
        next_request_time = in.readLong();
        time_so_far = in.readLong();
        bytes_xferred = in.readLong();
        xfer_speed = in.readFloat();
        project_backoff = in.readLong();

        boolean[] bArray = in.createBooleanArray();
        generated_locally = bArray[0];
        xfer_active = bArray[1];
        is_upload = bArray[2];
    }

    public static final Parcelable.Creator<Transfer> CREATOR = new Parcelable.Creator<Transfer>() {
        public Transfer createFromParcel(Parcel in) {
            return new Transfer(in);
        }

        public Transfer[] newArray(int size) {
            return new Transfer[size];
        }
    };
}
