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

import android.os.Build.VERSION
import android.os.Build.VERSION_CODES
import android.os.Parcel
import android.os.Parcelable
import androidx.core.os.ParcelCompat.readBoolean
import androidx.core.os.ParcelCompat.writeBoolean
import java.io.Serializable

data class ProjectAttachReply(var errorNum: Int = 0, val messages: MutableList<String> = mutableListOf())

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
        var platforms: MutableList<PlatformInfo?> = mutableListOf(),
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
                    parcel.readString() ?: "", parcel.readString() ?: "",
                    parcel.readInt(), parcel.readInt(), parcel.readString() ?: "") {
        platforms = arrayListOf<PlatformInfo?>().apply {
            if (VERSION.SDK_INT >= VERSION_CODES.TIRAMISU) {
                parcel.readList(this as MutableList<PlatformInfo?>, PlatformInfo::class.java.classLoader, PlatformInfo::class.java)
            } else {
                @Suppress("DEPRECATION")
                parcel.readList(this as MutableList<*>, PlatformInfo::class.java.classLoader)
            }
        }
        termsOfUse = parcel.readString()

        usesName = readBoolean(parcel)
        webStopped = readBoolean(parcel)
        schedulerStopped = readBoolean(parcel)
        accountCreationDisabled = readBoolean(parcel)
        clientAccountCreationDisabled = readBoolean(parcel)
        accountManager = readBoolean(parcel)
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
        dest.writeList(platforms.toList())
        dest.writeString(termsOfUse)

        writeBoolean(dest, usesName)
        writeBoolean(dest, webStopped)
        writeBoolean(dest, schedulerStopped)
        writeBoolean(dest, accountCreationDisabled)
        writeBoolean(dest, clientAccountCreationDisabled)
        writeBoolean(dest, accountManager)
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
                    if (VERSION.SDK_INT >= VERSION_CODES.TIRAMISU) {
                        parcel.readSerializable(null, ArrayList::class.java) as ArrayList<String>
                    } else {
                        @Suppress("DEPRECATION")
                        parcel.readSerializable() as ArrayList<String>
                    }, parcel.readString(), parcel.readString())

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
        const val IMAGE_URL = "image"
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
