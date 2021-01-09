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

/**
 * Account credentials, consisting of a [url], [emailAddress], [userName], [password], [teamName]
 * and [usesName] (if true, ID represents a user name, otherwise the user's email address).
 */
@Parcelize
data class AccountIn(
    var url: String? = null,
    var emailAddress: String? = null,
    var userName: String? = null,
    var password: String? = null,
    var teamName: String? = null,
    var usesName: Boolean = false
) : Parcelable

@Parcelize
data class AccountOut(var errorNum: Int = 0, var errorMsg: String? = "", var authenticator: String? = "") : Parcelable {
    object Fields {
        const val ERROR_MSG = "error_msg"
        const val AUTHENTICATOR = "authenticator"
    }
}
