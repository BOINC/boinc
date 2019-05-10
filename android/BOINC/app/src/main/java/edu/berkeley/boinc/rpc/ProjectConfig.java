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

import java.util.ArrayList;

import android.os.Parcel;
import android.os.Parcelable;

// according to http://boinc.berkeley.edu/trac/wiki/WebRpc

public class ProjectConfig implements Parcelable {
    public Integer error_num = 0; // if results are not present yet. (polling)
    public String name = "";
    public String masterUrl = "";
    public String webRpcUrlBase = "";
    public String localRevision = ""; // e.g. 4.3.2 can't be parse as int or float.
    public Integer minPwdLength = 0;
    public Boolean usesName = false;
    public Boolean webStopped = false;
    public Boolean schedulerStopped = false;
    public Boolean accountCreationDisabled = false;
    public Boolean clientAccountCreationDisabled = false;
    public Boolean accountManager = false;
    public Integer minClientVersion = 0;
    public String rpcPrefix = "";
    public ArrayList<PlatformInfo> platforms = new ArrayList<>();
    public String termsOfUse;

    /**
     * Returns the URL for HTTPS requests, if available.
     * master URL otherwise.
     * Use HTTPS URL for account look up and registration
     * CAUTION: DO NOT use HTTPS URL for attach!
     *
     * @return URL for accoutn look up and registration RPCs
     */
    public String getSecureUrlIfAvailable() {
        if(webRpcUrlBase != null && !webRpcUrlBase.isEmpty()) {
            return webRpcUrlBase;
        }
        else {
            return masterUrl;
        }
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        // TODO Auto-generated method stub
        dest.writeInt(error_num);
        dest.writeString(name);
        dest.writeString(masterUrl);
        dest.writeString(webRpcUrlBase);
        dest.writeString(localRevision);
        dest.writeInt(minPwdLength);
        dest.writeInt(minClientVersion);
        dest.writeString(rpcPrefix);
        dest.writeList(platforms);
        dest.writeString(termsOfUse);

        dest.writeBooleanArray(new boolean[]{
                usesName,
                webStopped,
                schedulerStopped,
                accountCreationDisabled,
                clientAccountCreationDisabled,
                accountManager});

    }

    public ProjectConfig() {
    }

    private ProjectConfig(Parcel in) {
        error_num = in.readInt();
        name = in.readString();
        masterUrl = in.readString();
        webRpcUrlBase = in.readString();
        localRevision = in.readString();
        minPwdLength = in.readInt();
        minClientVersion = in.readInt();
        rpcPrefix = in.readString();
        in.readList(platforms, PlatformInfo.class.getClassLoader());
        termsOfUse = in.readString();

        boolean[] bArray = in.createBooleanArray();
        usesName = bArray[0];
        webStopped = bArray[1];
        schedulerStopped = bArray[2];
        accountCreationDisabled = bArray[3];
        clientAccountCreationDisabled = bArray[4];
        accountManager = bArray[5];
    }

    public static final Parcelable.Creator<ProjectConfig> CREATOR = new Parcelable.Creator<ProjectConfig>() {
        public ProjectConfig createFromParcel(Parcel in) {
            return new ProjectConfig(in);
        }

        public ProjectConfig[] newArray(int size) {
            return null;
        }
    };
}
