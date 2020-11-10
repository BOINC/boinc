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

data class Message(
        var project: String = "",
        var priority: Int = 0,
        var seqno: Int = -1,
        var timestamp: Long = 0,
        var body: String? = null
) : Serializable, Parcelable {
    private constructor(parcel: Parcel) :
            this(parcel.readString() ?: "", parcel.readInt(), parcel.readInt(), parcel.readLong(),
                    parcel.readString()?.removePrefix("<![CDATA[")?.removeSuffix("]]>"))

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, flags: Int) {
        dest.writeString(project)
        dest.writeInt(priority)
        dest.writeInt(seqno)
        dest.writeLong(timestamp)
        dest.writeString(body)
    }

    object Fields {
        const val PRIORITY = "pri"
        const val TIMESTAMP = "time"
        const val BODY = "body"
    }

    companion object {
        private const val serialVersionUID = 1L

        @JvmField
        val CREATOR: Parcelable.Creator<Message> = object : Parcelable.Creator<Message> {
            override fun createFromParcel(parcel: Parcel) = Message(parcel)

            override fun newArray(size: Int) = arrayOfNulls<Message>(size)
        }
    }
}
