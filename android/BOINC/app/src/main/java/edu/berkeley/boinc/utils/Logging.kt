/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2021 University of California
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
package edu.berkeley.boinc.utils

object Logging {
    const val TAG = "BOINC_GUI"
    const val WAKELOCK = TAG + ":MyPowerLock"
    @JvmField
    var LOGLEVEL = -1
    @JvmField
    var ERROR = false
    @JvmField
    var WARNING = false
    @JvmField
    var INFO = false
    @JvmField
    var DEBUG = false
    @JvmField
    var VERBOSE = false
    @JvmField
    var RPC_PERFORMANCE = false
    @JvmField
    var RPC_DATA = false
    @JvmStatic
    fun setLogLevel(logLevel: Int) {
        LOGLEVEL = logLevel
        ERROR = LOGLEVEL > 0
        WARNING = LOGLEVEL > 1
        INFO = LOGLEVEL > 2
        DEBUG = LOGLEVEL > 3
        VERBOSE = LOGLEVEL > 4
    }
}
