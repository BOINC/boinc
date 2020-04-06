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

import android.graphics.Bitmap
import android.os.Parcel
import android.os.Parcelable

class ImageWrapper(var image: Bitmap?, var projectName: String?, var path: String?) : Parcelable {
    private constructor(parcel: Parcel) : this(parcel.readValue(Bitmap::class.java.classLoader) as Bitmap?,
            parcel.readString(), parcel.readString())

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, arg1: Int) {
        dest.writeValue(image)
        dest.writeString(projectName)
        dest.writeString(path)
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<ImageWrapper> = object : Parcelable.Creator<ImageWrapper> {
            override fun createFromParcel(parcel: Parcel) = ImageWrapper(parcel)

            override fun newArray(size: Int) = arrayOfNulls<ImageWrapper>(size)
        }
    }
}
