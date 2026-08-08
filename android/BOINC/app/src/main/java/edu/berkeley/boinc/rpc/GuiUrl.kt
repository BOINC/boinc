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

data class GuiUrl(var name: String? = "", var description: String? = "", var url: String? = "") : Parcelable {
    private constructor(parcel: Parcel) : this(parcel.readString(), parcel.readString(), parcel.readString())

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, arg1: Int) {
        dest.writeString(name)
        dest.writeString(description)
        dest.writeString(url)
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<GuiUrl> = object : Parcelable.Creator<GuiUrl> {
            override fun createFromParcel(parcel: Parcel) = GuiUrl(parcel)

            override fun newArray(size: Int) = arrayOfNulls<GuiUrl>(size)
        }
    }
}
