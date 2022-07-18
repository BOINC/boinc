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
import java.util.*

data class App
@JvmOverloads // generates overloaded constructors
constructor(
        var name: String? = "",
        var userFriendlyName: String? = "",
        var nonCpuIntensive: Int = 0,
        var project: Project? = null
) : Parcelable {
    private constructor(parcel: Parcel) : this(
            parcel.readString(),
            parcel.readString(),
            parcel.readInt(),
            parcel.readValue(Project::class.java.classLoader) as Project?
    )

    val displayName: String?
        get() = if (userFriendlyName.isNullOrEmpty()) name else userFriendlyName

    override fun equals(other: Any?): Boolean {
        if (this === other) return true

        return other is App && name.equals(other.name, ignoreCase = true) &&
                userFriendlyName.equals(other.userFriendlyName, ignoreCase = true) &&
                nonCpuIntensive == other.nonCpuIntensive && project == other.project
    }

    override fun hashCode(): Int {
        var result = name?.lowercase(Locale.ROOT).hashCode()
        result = 31 * result + userFriendlyName?.lowercase(Locale.ROOT).hashCode()
        result = 31 * result + nonCpuIntensive
        result = 31 * result + project.hashCode()
        return result
    }

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, flags: Int) {
        dest.writeString(name)
        dest.writeString(userFriendlyName)
        dest.writeInt(nonCpuIntensive)
        dest.writeValue(project)
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<App> = object : Parcelable.Creator<App> {
            override fun createFromParcel(parcel: Parcel) = App(parcel)

            override fun newArray(size: Int) = arrayOfNulls<App>(size)
        }
    }
}
