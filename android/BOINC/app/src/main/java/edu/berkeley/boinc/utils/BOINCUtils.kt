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
@file:JvmName("BOINCUtils")

package edu.berkeley.boinc.utils

import android.content.Context
import edu.berkeley.boinc.R
import java.io.IOException
import java.io.Reader

@Throws(IOException::class)
fun Reader.readLineLimit(limit: Int): String? {
    val sb = StringBuilder()
    for (i in 0 until limit) {
        val c = read() //Read in single character
        if (c == -1) {
            return if (sb.isNotEmpty()) sb.toString() else null
        }
        if (c.toChar() == '\n' || c.toChar() == '\r') { //Found end of line, break loop.
            break
        }
        sb.append(c.toChar()) // String is not over and end line not found
    }
    return sb.toString() //end of line was found.
}

fun Context.translateRPCReason(reason: Int) = when (reason) {
    BOINCDefs.RPC_REASON_USER_REQ -> resources.getString(R.string.rpcreason_userreq)
    BOINCDefs.RPC_REASON_NEED_WORK -> resources.getString(R.string.rpcreason_needwork)
    BOINCDefs.RPC_REASON_RESULTS_DUE -> resources.getString(R.string.rpcreason_resultsdue)
    BOINCDefs.RPC_REASON_TRICKLE_UP -> resources.getString(R.string.rpcreason_trickleup)
    BOINCDefs.RPC_REASON_ACCT_MGR_REQ -> resources.getString(R.string.rpcreason_acctmgrreq)
    BOINCDefs.RPC_REASON_INIT -> resources.getString(R.string.rpcreason_init)
    BOINCDefs.RPC_REASON_PROJECT_REQ -> resources.getString(R.string.rpcreason_projectreq)
    else -> resources.getString(R.string.rpcreason_unknown)
}
