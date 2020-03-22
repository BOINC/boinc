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

/**
 * Holds information about the currently used account manager.
 */
@EqualsAndHashCode
@FieldNameConstants
@ToString
public class AcctMgrInfo implements Parcelable {
    public String acct_mgr_name;
    public String acct_mgr_url;
    public String cookie_failure_url;
    public boolean have_credentials;
    public boolean cookie_required;
    @EqualsAndHashCode.Exclude public boolean present;

    @Override
    public int describeContents() {
        return 0;
    }

    AcctMgrInfo(String acct_mgr_name, String acct_mgr_url, String cookie_failure_url,
                        boolean have_credentials, boolean cookie_required, boolean present) {
        this.acct_mgr_name = acct_mgr_name;
        this.acct_mgr_url = acct_mgr_url;
        this.cookie_failure_url = cookie_failure_url;
        this.have_credentials = have_credentials;
        this.cookie_required = cookie_required;
        this.present = present;
    }

    AcctMgrInfo(String acct_mgr_name) {
        this(acct_mgr_name, "", "", false, false, false);
    }

    AcctMgrInfo(String acct_mgr_name, String acct_mgr_url) {
        this(acct_mgr_name, acct_mgr_url, "", false, false, false);
    }

    AcctMgrInfo(String acct_mgr_name, String acct_mgr_url, String cookie_failure_url) {
        this(acct_mgr_name, acct_mgr_url, cookie_failure_url, false, false, false);
    }

    public AcctMgrInfo() {
        this("", "", "", false, false, false);
    }

    private AcctMgrInfo(Parcel in) {
        this(in.readString(), in.readString(), in.readString());

        boolean[] bArray = in.createBooleanArray();
        have_credentials = bArray[0];
        cookie_required = bArray[1];
        present = bArray[2];
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

    public static final Parcelable.Creator<AcctMgrInfo> CREATOR =
            new Parcelable.Creator<AcctMgrInfo>() {
                public AcctMgrInfo createFromParcel(Parcel in) {
                    return new AcctMgrInfo(in);
                }

                public AcctMgrInfo[] newArray(int size) {
                    return null;
                }
            };
}
