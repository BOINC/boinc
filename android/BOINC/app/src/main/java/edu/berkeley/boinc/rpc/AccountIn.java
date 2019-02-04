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

public class AccountIn implements Parcelable {
    public String url; // either master_url or web_rpc_url_base (HTTPS), if present
    public String email_addr;
    public String user_name;
    public Boolean uses_name;
    public String passwd;
    public String team_name;

    public static final Parcelable.Creator<AccountIn> CREATOR = new Parcelable.Creator<AccountIn>() {
        public AccountIn createFromParcel(Parcel in) {
            return new AccountIn(in);
        }

        public AccountIn[] newArray(int size) {
            return null;
        }
    };

    public AccountIn() {
        super();
    }

    /**
     * Account credentials
     *
     * @param url      URL of project, either masterUrl(HTTP) or webRpcUrlBase(HTTPS)
     * @param email    email address of user
     * @param userName user name
     * @param usesName if true, id represents a user name, if not, the user's email address
     * @param password password
     * @param teamName name of team, account shall get associated to
     */
    public AccountIn(String url, String email, String userName, Boolean usesName, String password, String teamName) {
        this.url = url;
        this.email_addr = email;
        this.user_name = userName;
        this.uses_name = usesName;
        this.passwd = password;
        this.team_name = teamName;
    }

    private AccountIn(Parcel in) {
        readFromParcel(in);
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeString(url);
        out.writeString(email_addr);
        out.writeString(user_name);
        out.writeString(passwd);
        out.writeString(team_name);
        out.writeBooleanArray(new boolean[]{
                uses_name});
    }

    public void readFromParcel(Parcel in) {
        url = in.readString();
        email_addr = in.readString();
        user_name = in.readString();
        passwd = in.readString();
        team_name = in.readString();
        boolean[] bArray = in.createBooleanArray();
        uses_name = bArray[0];
    }
}
