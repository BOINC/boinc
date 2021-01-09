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
 * Holds information about the attachable account managers.
 * The source of the account managers is all_projects_list.xml.
 */
@Parcelize
data class AccountManager(
    var name: String = "",
    var url: String = "",
    var description: String = "",
    var imageUrl: String = ""
) : Parcelable

/**
 * Holds information about the currently used account manager.
 */
@Parcelize
data class AcctMgrInfo @JvmOverloads constructor(
    var acctMgrName: String = "",
    var acctMgrUrl: String = "",
    var isHavingCredentials: Boolean = false,
    var isPresent: Boolean = false
) : Parcelable {
    object Fields {
        const val ACCT_MGR_NAME = "acct_mgr_name"
        const val ACCT_MGR_URL = "acct_mgr_url"
        const val HAVING_CREDENTIALS = "have_credentials"
    }
}

data class AcctMgrRPCReply(var errorNum: Int = 0, val messages: MutableList<String> = arrayListOf())
