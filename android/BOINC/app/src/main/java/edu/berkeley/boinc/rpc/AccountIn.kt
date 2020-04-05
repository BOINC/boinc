/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2020 University of California
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
package edu.berkeley.boinc.rpc

import android.os.Build
import android.os.Parcel
import android.os.Parcelable

/**
 * Account credentials
 *
 * @param url      URL of project, either masterUrl(HTTP) or webRpcUrlBase(HTTPS)
 * @param email    email address of user
 * @param userName user name
 * @param password password
 * @param teamName name of team, account shall get associated to
 * @param usesName if true, id represents a user name, if not, the user's email address
 */
class AccountIn(
        var url: String? = null,
        var emailAddress: String? = null,
        var userName: String? = null,
        var password: String? = null,
        var teamName: String? = null,
        var usesName: Boolean = false
) : Parcelable {
    private constructor(parcel: Parcel) :
            this(parcel.readString(), parcel.readString(), parcel.readString(), parcel.readString(),
                    parcel.readString()) {
        usesName = if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            parcel.createBooleanArray()!![0]
        } else {
            parcel.readBoolean()
        }
    }

    override fun describeContents() = 0

    override fun writeToParcel(parcel: Parcel, flags: Int) {
        parcel.writeString(url)
        parcel.writeString(emailAddress)
        parcel.writeString(userName)
        parcel.writeString(password)
        parcel.writeString(teamName)
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            parcel.writeBooleanArray(booleanArrayOf(usesName))
        } else {
            parcel.writeBoolean(usesName)
        }
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<AccountIn> = object : Parcelable.Creator<AccountIn> {
            override fun createFromParcel(parcel: Parcel) = AccountIn(parcel)

            override fun newArray(size: Int) = arrayOfNulls<AccountIn>(size)
        }
    }
}
