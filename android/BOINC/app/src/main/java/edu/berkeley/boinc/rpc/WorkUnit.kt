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

data class WorkUnit(
        var name: String = "",
        var appName: String = "",
        var versionNum: Int = 0,
        var rscFloatingPointOpsEst: Double = 0.0,
        var rscFloatingPointOpsBound: Double = 0.0,
        var rscMemoryBound: Double = 0.0,
        var rscDiskBound: Double = 0.0,
        var project: Project? = null,
        var app: App? = null
) : Parcelable {
    private constructor(parcel: Parcel) :
            this(parcel.readString() ?: "", parcel.readString() ?: "",
                    parcel.readInt(), parcel.readDouble(), parcel.readDouble(), parcel.readDouble(),
                    parcel.readDouble(), parcel.readValue(Project::class.java.classLoader) as Project?,
                    parcel.readValue(App::class.java.classLoader) as App?)

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, flags: Int) {
        dest.writeString(name)
        dest.writeString(appName)
        dest.writeInt(versionNum)
        dest.writeDouble(rscFloatingPointOpsEst)
        dest.writeDouble(rscFloatingPointOpsBound)
        dest.writeDouble(rscMemoryBound)
        dest.writeDouble(rscDiskBound)
        dest.writeValue(project)
        dest.writeValue(app)
    }

    object Fields {
        const val APP_NAME = "app_name"
        const val VERSION_NUM = "version_num"
        const val RSC_FPOPS_EST = "rsc_fpops_est"
        const val RSC_FPOPS_BOUND = "rsc_fpops_bound"
        const val RSC_MEMORY_BOUND = "rsc_memory_bound"
        const val RSC_DISK_BOUND = "rsc_disk_bound"
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<WorkUnit> = object : Parcelable.Creator<WorkUnit> {
            override fun createFromParcel(parcel: Parcel) = WorkUnit(parcel)

            override fun newArray(size: Int) = arrayOfNulls<WorkUnit>(size)
        }
    }
}
