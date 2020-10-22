/**
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
 * along with BOINC.  If not, see <http:></http:>//www.gnu.org/licenses/>.
 */
package edu.berkeley.boinc.utils

import android.os.Parcel
import android.os.Parcelable

data class ErrorCodeDescription @JvmOverloads constructor(val code: Int = 0, val description: String? = "")
    : Parcelable {
    private constructor(parcel: Parcel) : this(parcel.readInt(), parcel.readString())

    val isOK: Boolean
        get() = code == ERR_OK && description.isNullOrEmpty()

    override fun describeContents() = 0

    override fun writeToParcel(parcel: Parcel, flags: Int) {
        parcel.writeInt(code)
        parcel.writeString(description)
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<ErrorCodeDescription> = object : Parcelable.Creator<ErrorCodeDescription> {
            override fun createFromParcel(parcel: Parcel) = ErrorCodeDescription(parcel)

            override fun newArray(size: Int) = arrayOfNulls<ErrorCodeDescription>(size)
        }
    }
}
