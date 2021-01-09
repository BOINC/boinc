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

import android.os.Parcelable
import kotlinx.parcelize.Parcelize
import java.io.Serializable

@Parcelize
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
}
