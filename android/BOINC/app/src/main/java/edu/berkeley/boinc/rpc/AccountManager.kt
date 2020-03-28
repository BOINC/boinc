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
            this(parcel.readString() ?: "",
                    parcel.readString() ?: "",
                    parcel.readString() ?: "",
                    parcel.readString() ?: "")

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, arg1: Int) {
        dest.writeString(name)
        dest.writeString(url)
        dest.writeString(description)
        dest.writeString(imageUrl)
    }

    object Fields {
        const val NAME = "name"
        const val URL = "url"
        const val DESCRIPTION = "description"
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<AccountManager> = object : Parcelable.Creator<AccountManager> {
            override fun createFromParcel(parcel: Parcel) = AccountManager(parcel)

            override fun newArray(size: Int) = arrayOfNulls<AccountManager>(size)
        }
    }
}
