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

import android.util.Log

object Logging {
    const val TAG = "BOINC_GUI"
    const val WAKELOCK = "$TAG:MyPowerLock"
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
    var categories = HashSet<CATEGORY>()

    @JvmStatic
    fun setLogLevel(logLevel: Int) {
        LOGLEVEL = logLevel
        ERROR = LOGLEVEL > 0
        WARNING = LOGLEVEL > 1
        INFO = LOGLEVEL > 2
        DEBUG = LOGLEVEL > 3
        VERBOSE = LOGLEVEL > 4
    }

    @JvmStatic
    fun setLogCategory(logCategory: String, value: Boolean) {
        try {
            val key = CATEGORY.valueOf(logCategory)

            if (value && key !in categories) {
                categories.add(key)
            } else if (!value && key in categories) {
                categories.remove(key)
            }
        }
        catch (e: Exception) {
            logError(CATEGORY.SETTINGS, "Wrong settings key: $logCategory")
        }
    }

    @JvmStatic
    fun isLoggable(logLevel: LEVEL, logCategory: CATEGORY): Boolean {
        return logLevel.logLevel <= LOGLEVEL && logCategory in categories
    }

    @JvmStatic
    fun logMessage(logLevel: LEVEL, logCategory: CATEGORY, logMessage: String, e: Throwable? = null) {
        if (!isLoggable(logLevel, logCategory)) return

        val message = "[$logCategory] $logMessage"
        when (logLevel) {
            LEVEL.ERROR -> Log.e(TAG, message, e)
            LEVEL.WARNING -> Log.w(TAG, message)
            LEVEL.INFO -> Log.i(TAG, message)
            LEVEL.DEBUG -> Log.d(TAG, message)
            LEVEL.VERBOSE -> Log.v(TAG, message)
        }
    }

    @JvmStatic
    fun logException(logCategory: CATEGORY, logMessage: String, e: Throwable) {
        logMessage(LEVEL.ERROR, logCategory, logMessage, e)
    }
    @JvmStatic
    fun logError(logCategory: CATEGORY, logMessage: String) {
        logMessage(LEVEL.ERROR, logCategory, logMessage)
    }
    @JvmStatic
    fun logWarning(logCategory: CATEGORY, logMessage: String) {
        logMessage(LEVEL.WARNING, logCategory, logMessage)
    }
    @JvmStatic
    fun logInfo(logCategory: CATEGORY, logMessage: String) {
        logMessage(LEVEL.INFO, logCategory, logMessage)
    }
    @JvmStatic
    fun logDebug(logCategory: CATEGORY, logMessage: String) {
        logMessage(LEVEL.DEBUG, logCategory, logMessage)
    }
    @JvmStatic
    fun logVerbose(logCategory: CATEGORY, logMessage: String) {
        logMessage(LEVEL.VERBOSE, logCategory, logMessage)
    }

    enum class LEVEL(val logLevel: Int) {
        ERROR(1),
        WARNING(2),
        INFO(3),
        DEBUG(4),
        VERBOSE(5)
    }

    enum class CATEGORY {
        USER_ACTION,
        GUI_VIEW,
        MONITOR,
        TASKS,
        PROJECTS,
        GUI_ACTIVITY,
        PROJECT_SERVICE,
        SETTINGS,
        CLIENT,
        DEVICE,
        RPC,
        XML
    }
}
