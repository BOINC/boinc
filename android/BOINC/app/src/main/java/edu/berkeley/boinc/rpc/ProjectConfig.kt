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

// according to http://boinc.berkeley.edu/trac/wiki/WebRpc
data class ProjectConfig(
        var errorNum: Int = 0, // if results are not present yet (polling)
        var name: String = "",
        var masterUrl: String = "",
        var webRpcUrlBase: String = "",
        var localRevision: String = "", // e.g. 4.3.2 can't be parsed as int or float.
        var minPwdLength: Int = 0,
        var minClientVersion: Int = 0,
        var rpcPrefix: String = "",
        var platforms: List<PlatformInfo?> = mutableListOf(),
        var termsOfUse: String? = null,
        var usesName: Boolean = false,
        var webStopped: Boolean = false,
        var schedulerStopped: Boolean = false,
        var accountCreationDisabled: Boolean = false,
        var clientAccountCreationDisabled: Boolean = false,
        var accountManager: Boolean = false
) : Parcelable {
    private constructor(parcel: Parcel) :
            this(parcel.readInt(), parcel.readString() ?: "", parcel.readString() ?: "",
                    parcel.readString() ?: "", parcel.readString() ?: "", parcel.readInt(),
                    parcel.readInt(), parcel.readString() ?: "") {
        parcel.readList(platforms, PlatformInfo::class.java.classLoader)
        termsOfUse = parcel.readString()
        val bArray = parcel.createBooleanArray()!!
        usesName = bArray[0]
        webStopped = bArray[1]
        schedulerStopped = bArray[2]
        accountCreationDisabled = bArray[3]
        clientAccountCreationDisabled = bArray[4]
        accountManager = bArray[5]
    }

    /**
     * Returns the URL for HTTPS requests, if available; otherwise the master URL is returned.
     * Use HTTPS URL for account lookup and registration.
     * CAUTION: DO NOT use HTTPS URL for attach!
     *
     * @return URL for account lookup and registration RPCs
     */
    val secureUrlIfAvailable: String?
        get() = if (webRpcUrlBase.isNotEmpty()) webRpcUrlBase else masterUrl

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, flags: Int) {
        dest.writeInt(errorNum)
        dest.writeString(name)
        dest.writeString(masterUrl)
        dest.writeString(webRpcUrlBase)
        dest.writeString(localRevision)
        dest.writeInt(minPwdLength)
        dest.writeInt(minClientVersion)
        dest.writeString(rpcPrefix)
        dest.writeList(platforms)
        dest.writeString(termsOfUse)
        dest.writeBooleanArray(booleanArrayOf(usesName, webStopped, schedulerStopped,
                accountCreationDisabled, clientAccountCreationDisabled, accountManager))
    }

    object Fields {
        const val WEB_RPC_URL_BASE = "web_rpc_url_base"
        const val LOCAL_REVISION = "local_revision"
        const val MIN_PWD_LENGTH = "min_pwd_length"
        const val WEB_STOPPED = "web_stopped"
        const val SCHEDULER_STOPPED = "scheduler_stopped"
        const val CLIENT_ACCOUNT_CREATION_DISABLED = "client_account_creation_disabled"
        const val ACCOUNT_MANAGER = "account_manager"
        const val MIN_CLIENT_VERSION = "min_client_version"
        const val RPC_PREFIX = "rpc_prefix"
        const val PLATFORMS = "platforms"
        const val TERMS_OF_USE = "terms_of_use"
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<ProjectConfig> = object : Parcelable.Creator<ProjectConfig> {
            override fun createFromParcel(parcel: Parcel) = ProjectConfig(parcel)

            override fun newArray(size: Int) = arrayOfNulls<ProjectConfig>(size)
        }
    }
}
