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
import androidx.core.os.ParcelCompat.readBoolean
import androidx.core.os.ParcelCompat.writeBoolean
import java.io.Serializable

enum class TransferStatus(val status: Int) {
    ERR_GIVEUP_DOWNLOAD(-114),
    ERR_GIVEUP_UPLOAD(-115)
}

data class Transfer(
        var name: String = "",
        var projectUrl: String = "",
        var noOfBytes: Long = 0,
        var status: Int = 0,
        var nextRequestTime: Long = 0,
        var timeSoFar: Long = 0,
        var bytesTransferred: Long = 0,
        var transferSpeed: Float = 0f,
        var projectBackoff: Long = 0,
        var generatedLocally: Boolean = false,
        var isTransferActive: Boolean = false,
        var isUpload: Boolean = false
) : Serializable, Parcelable {
    private constructor (parcel: Parcel) :
            this(parcel.readString() ?: "", parcel.readString() ?: "",
                    parcel.readLong(), parcel.readInt(), parcel.readLong(), parcel.readLong(),
                    parcel.readLong(), parcel.readFloat(), parcel.readLong(), readBoolean(parcel),
                    readBoolean(parcel), readBoolean(parcel))

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, flags: Int) {
        dest.writeString(name)
        dest.writeString(projectUrl)
        dest.writeLong(noOfBytes)
        dest.writeInt(status)
        dest.writeLong(nextRequestTime)
        dest.writeLong(timeSoFar)
        dest.writeLong(bytesTransferred)
        dest.writeFloat(transferSpeed)
        dest.writeLong(projectBackoff)
        writeBoolean(dest, generatedLocally)
        writeBoolean(dest, isTransferActive)
        writeBoolean(dest, isUpload)
    }

    object Fields {
        const val GENERATED_LOCALLY = "generated_locally"
        const val NBYTES = "nbytes"
        const val IS_UPLOAD = "is_upload"
        const val STATUS = "status"
        const val NEXT_REQUEST_TIME = "next_request_time"
        const val TIME_SO_FAR = "time_so_far"
        const val BYTES_XFERRED = "bytes_xferred"
        const val XFER_SPEED = "xfer_speed"
        const val PROJECT_BACKOFF = "project_backoff"
    }

    companion object {
        private const val serialVersionUID = 1L

        @JvmField
        val CREATOR: Parcelable.Creator<Transfer> = object : Parcelable.Creator<Transfer> {
            override fun createFromParcel(parcel: Parcel) = Transfer(parcel)

            override fun newArray(size: Int) = arrayOfNulls<Transfer>(size)
        }
    }
}
