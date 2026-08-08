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

data class AppVersion(
        var appName: String? = null,
        var versionNum: Int = 0,
        var platform: String? = null,
        var planClass: String? = null,
        var apiVersion: String? = null,
        var avgNoOfCPUs: Double = 0.0,
        var maxNoOfCPUs: Double = 0.0,
        var gpuRam: Double = 0.0,
        var app: App? = null,
        var project: Project? = null
) : Parcelable {
    internal constructor(appName: String?, versionNum: Int) : this(appName) {
        this.versionNum = versionNum
    }

    private constructor(parcel: Parcel) : this(
            parcel.readString(),
            parcel.readInt(),
            parcel.readString(),
            parcel.readString(),
            parcel.readString(),
            parcel.readDouble(),
            parcel.readDouble(),
            parcel.readDouble(),
            parcel.readValue(App::class.java.classLoader) as App?,
            parcel.readValue(Project::class.java.classLoader) as Project?
    )

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, flags: Int) {
        dest.writeString(appName)
        dest.writeInt(versionNum)
        dest.writeString(platform)
        dest.writeString(planClass)
        dest.writeString(apiVersion)
        dest.writeDouble(avgNoOfCPUs)
        dest.writeDouble(maxNoOfCPUs)
        dest.writeDouble(gpuRam)
        dest.writeValue(app)
        dest.writeValue(project)
    }

    object Fields {
        const val APP_NAME = "app_name"
        const val VERSION_NUM = "version_num"
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<AppVersion> = object : Parcelable.Creator<AppVersion> {
            override fun createFromParcel(parcel: Parcel) = AppVersion(parcel)

            override fun newArray(size: Int) = arrayOfNulls<AppVersion>(size)
        }
    }
}
