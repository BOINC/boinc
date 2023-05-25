/*******************************************************************************
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
 ******************************************************************************/
package edu.berkeley.boinc.rpc;

import android.os.Parcel;
import android.os.Parcelable;

public class AccountOut implements Parcelable {
	public int error_num = 0;
	public String error_msg = "";
	public String authenticator = "";

	public static final Parcelable.Creator<AccountOut> CREATOR = new Parcelable.Creator<AccountOut>() {
		public AccountOut createFromParcel(Parcel in) {
		    return new AccountOut(in);
		}
		public AccountOut[] newArray(int size) {
		    return null;
		}
	};

	public AccountOut() {
		super();
	}

	private AccountOut(Parcel in) {
	    readFromParcel(in);
	}

	public int describeContents() {
	    return 0;
	}

	public void writeToParcel(Parcel out, int flags) {
	    out.writeInt(error_num);
	    out.writeString(error_msg);
	    out.writeString(authenticator);
	}
	public void readFromParcel(Parcel in) {
		error_num = in.readInt();
		error_msg = in.readString();
		authenticator = in.readString();
	}
}
