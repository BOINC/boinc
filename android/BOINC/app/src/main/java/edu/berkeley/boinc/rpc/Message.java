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

public class Message implements Serializable, Parcelable {
    private static final long serialVersionUID = 1L;
    public static final int MSG_INFO = 1;
    public static final int MSG_USER_ALERT = 2;
    public static final int MSG_INTERNAL_ERROR = 3;
    // internally used by client
    public static final int MSG_SCHEDULER_ALERT = 4;

    public String project = "";
    public int priority;
    public int seqno;
    public long timestamp;
    public String body;

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        // TODO Auto-generated method stub
        dest.writeString(project);
        dest.writeInt(priority);
        dest.writeInt(seqno);
        dest.writeLong(timestamp);
        dest.writeString(body);
    }

    public Message() {
    }

    private Message(Parcel in) {
        project = in.readString();
        priority = in.readInt();
        seqno = in.readInt();
        timestamp = in.readLong();
        body = in.readString();
        body = body.replace("<![CDATA[", "");
        body = body.replace("]]>", "");
    }

    public static final Parcelable.Creator<Message> CREATOR = new Parcelable.Creator<Message>() {
        public Message createFromParcel(Parcel in) {
            return new Message(in);
        }

        public Message[] newArray(int size) {
            return new Message[size];
        }
    };
}
