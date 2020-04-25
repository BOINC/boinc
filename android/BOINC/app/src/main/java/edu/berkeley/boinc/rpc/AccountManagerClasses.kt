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

import android.os.Parcel
import android.os.Parcelable
import androidx.core.os.ParcelCompat.readBoolean
import androidx.core.os.ParcelCompat.writeBoolean

/**
 * Holds information about the attachable account managers.
 * The source of the account managers is all_projects_list.xml.
 */
data class AccountManager internal constructor(
        var name: String = "",
        var url: String = "",
        var description: String = "",
        var imageUrl: String = ""
) : Parcelable {
    private constructor(parcel: Parcel) :
            this(parcel.readString() ?: "", parcel.readString() ?: "",
                    parcel.readString() ?: "", parcel.readString() ?: "")

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, arg1: Int) {
        dest.writeString(name)
        dest.writeString(url)
        dest.writeString(description)
        dest.writeString(imageUrl)
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<AccountManager> = object : Parcelable.Creator<AccountManager> {
            override fun createFromParcel(parcel: Parcel) = AccountManager(parcel)

            override fun newArray(size: Int) = arrayOfNulls<AccountManager>(size)
        }
    }
}

/**
 * Holds information about the currently used account manager.
 */
data class AcctMgrInfo internal constructor(
        var acctMgrName: String = "",
        var acctMgrUrl: String = "",
        var cookieFailureUrl: String = "",
        var isHavingCredentials: Boolean = false,
        var isCookieRequired: Boolean = false
) : Parcelable {
    var isPresent: Boolean = false

    constructor(
            acctMgrName: String,
            acctMgrUrl: String,
            cookieFailureUrl: String,
            isHavingCredentials: Boolean,
            isCookieRequired: Boolean,
            isPresent: Boolean)
            : this(acctMgrName, acctMgrUrl, cookieFailureUrl, isHavingCredentials, isCookieRequired) {
        this.isPresent = isPresent
    }

    private constructor(parcel: Parcel) :
            this(parcel.readString() ?: "", parcel.readString() ?: "",
                    parcel.readString() ?: "", readBoolean(parcel), readBoolean(parcel),
                    readBoolean(parcel))

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, flags: Int) {
        dest.writeString(acctMgrName)
        dest.writeString(acctMgrUrl)
        dest.writeString(cookieFailureUrl)
        writeBoolean(dest, isHavingCredentials)
        writeBoolean(dest, isCookieRequired)
        writeBoolean(dest, isPresent)
    }

    object Fields {
        const val ACCT_MGR_NAME = "acctMgrName"
        const val ACCT_MGR_URL = "acctMgrUrl"
        const val COOKIE_FAILURE_URL = "cookieFailureUrl"
        const val HAVING_CREDENTIALS = "havingCredentials"
        const val COOKIE_REQUIRED = "cookieRequired"
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<AcctMgrInfo> = object : Parcelable.Creator<AcctMgrInfo> {
            override fun createFromParcel(parcel: Parcel) = AcctMgrInfo(parcel)

            override fun newArray(size: Int) = arrayOfNulls<AcctMgrInfo>(size)
        }
    }
}

data class AcctMgrRPCReply(var errorNum: Int = 0, val messages: MutableList<String> = arrayListOf())
