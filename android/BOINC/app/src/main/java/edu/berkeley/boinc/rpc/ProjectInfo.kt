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
import java.io.Serializable
import java.util.*

// needs to be serializable to be put into Activity start Intent
data class ProjectInfo(
        var name: String = "",
        var url: String = "",
        var generalArea: String? = null,
        var specificArea: String? = null,
        var description: String? = null,
        var home: String? = null,
        var platforms: List<String> = mutableListOf(),
        var imageUrl: String? = null,
        var summary: String? = null
) : Serializable, Parcelable {
    @Suppress("UNCHECKED_CAST")
    private constructor(parcel: Parcel) :
            this(parcel.readString() ?: "", parcel.readString() ?: "", parcel.readString(),
                    parcel.readString(), parcel.readString(), parcel.readString(),
                    parcel.readSerializable() as ArrayList<String>, parcel.readString(), parcel.readString())

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, arg1: Int) {
        dest.writeString(name)
        dest.writeString(url)
        dest.writeString(generalArea)
        dest.writeString(specificArea)
        dest.writeString(description)
        dest.writeString(home)
        dest.writeSerializable(ArrayList(platforms))
        dest.writeString(imageUrl)
        dest.writeString(summary)
    }

    object Fields {
        const val GENERAL_AREA = "general_area"
        const val SPECIFIC_AREA = "specific_area"
        const val HOME = "home"
        const val PLATFORMS = "platforms"
        const val IMAGE_URL = "image_url"
        const val SUMMARY = "summary"
    }

    companion object {
        private const val serialVersionUID = -5944047529950035455L // auto generated

        @JvmField
        val CREATOR: Parcelable.Creator<ProjectInfo> = object : Parcelable.Creator<ProjectInfo> {
            override fun createFromParcel(parcel: Parcel) = ProjectInfo(parcel)

            override fun newArray(size: Int) = arrayOfNulls<ProjectInfo>(size)
        }
    }
}
