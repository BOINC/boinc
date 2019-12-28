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
 * Holds information about the attachable account managers.
 * The source of the account managers is all_projects_list.xml.
 */
public class AccountManager implements Parcelable{
	public String name = "";
	public String url = "";
	public String description;
	public String imageUrl;

	@Override
	public String toString() {
		String platformString = "";
		return "AccountManager: " + name + " ; " + url + " ; " + " ; " + " ; " + description + " ; " + " ; " + platformString + " ; " + imageUrl;
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
		dest.writeString(description);
		dest.writeString(imageUrl);
	}

	public AccountManager() {}

	private AccountManager(Parcel in) {
		name = in.readString();
		url = in.readString();
		description = in.readString();
		imageUrl = in.readString();
	}

	public static final Creator<AccountManager> CREATOR = new Creator<AccountManager>() {
		public AccountManager createFromParcel(Parcel in) {
		    return new AccountManager(in);
		}
		public AccountManager[] newArray(int size) {
		    return new AccountManager[size];
		}
	};
}
