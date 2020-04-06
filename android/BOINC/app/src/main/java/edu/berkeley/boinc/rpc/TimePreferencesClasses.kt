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

data class TimeSpan(var startHour: Double = 0.0, var endHour: Double = 0.0)

data class TimePreferences(
        var startHour: Double = 0.0,
        var endHour: Double = 0.0,
        var weekPrefs: Array<TimeSpan?> = arrayOfNulls(7)
) : Parcelable {
    private constructor(parcel: Parcel): this() {
        val dArray = parcel.createDoubleArray()!!
        startHour = dArray[0]
        endHour = dArray[1]
        var i = 2
        while (i <= 14) {
            val index = i / 2 - 1
            if (dArray[i] != Double.NEGATIVE_INFINITY) {
                weekPrefs[index]!!.startHour = dArray[i]
                weekPrefs[index]!!.endHour = dArray[i + 1]
            } else {
                weekPrefs[index] = null
            }
            i += 2
        }
    }

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, flags: Int) {
        val weekPrefValues = mutableListOf(startHour, endHour)
        for (i in 0..6) {
            weekPrefValues.add(weekPrefs[i]?.startHour ?: Double.NEGATIVE_INFINITY)
            weekPrefValues.add(weekPrefs[i]?.endHour ?: Double.NEGATIVE_INFINITY)
        }

        dest.writeDoubleArray(weekPrefValues.toDoubleArray())
    }

    override fun equals(other: Any?): Boolean {
        if (this === other) return true

        return other is TimePreferences && startHour == other.startHour &&
                endHour == other.endHour && weekPrefs.contentEquals(other.weekPrefs)
    }

    override fun hashCode(): Int {
        var result = startHour.hashCode()
        result = 31 * result + endHour.hashCode()
        result = 31 * result + weekPrefs.contentHashCode()
        return result
    }

    object Fields {
        const val START_HOUR = "startHour"
        const val END_HOUR = "endHour"
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<TimePreferences> = object : Parcelable.Creator<TimePreferences> {
            override fun createFromParcel(parcel: Parcel) = TimePreferences(parcel)

            override fun newArray(size: Int) = arrayOfNulls<TimePreferences>(size)
        }
    }
}
