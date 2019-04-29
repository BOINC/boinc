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

/**
 * Holds information about the currently used account manager.
 */
public class AcctMgrInfo implements Parcelable {
    public String acct_mgr_name;
    public String acct_mgr_url;
    public boolean have_credentials;
    public boolean cookie_required;
    public String cookie_failure_url;

    public boolean present;

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(acct_mgr_name);
        dest.writeString(acct_mgr_url);
        dest.writeString(cookie_failure_url);

        dest.writeBooleanArray(new boolean[]{
                have_credentials,
                cookie_required,
                present});

    }

    public AcctMgrInfo() {
    }

    private AcctMgrInfo(Parcel in) {
        acct_mgr_name = in.readString();
        acct_mgr_url = in.readString();
        cookie_failure_url = in.readString();

        boolean[] bArray = in.createBooleanArray();
        have_credentials = bArray[0];
        cookie_required = bArray[1];
        present = bArray[2];
    }

    public static final Parcelable.Creator<AcctMgrInfo> CREATOR = new Parcelable.Creator<AcctMgrInfo>() {
        public AcctMgrInfo createFromParcel(Parcel in) {
            return new AcctMgrInfo(in);
        }

        public AcctMgrInfo[] newArray(int size) {
            return null;
        }
    };
}
