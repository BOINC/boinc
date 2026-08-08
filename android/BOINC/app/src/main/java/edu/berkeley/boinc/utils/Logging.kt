/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2022 University of California
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

    private var logLevel = -1
    private var categories = HashSet<Category>()

    @JvmStatic
    fun getLogLevel(): Int {
        return logLevel
    }

    @JvmStatic
    fun setLogLevel(level: Int) {
        logLevel = level
    }

    @JvmStatic
    fun getLogCategories(): HashSet<Category> {
        return categories
    }

    @JvmStatic
    fun setLogCategories(categories: List<String>) {
        enumValues<Category>().forEach {
            setLogCategory(it.name, categories.contains(it.name))
        }
    }

    @JvmStatic
    fun setLogCategory(logCategory: String, value: Boolean) {
        try {
            val key = Category.valueOf(logCategory)

            if (value && key !in categories) {
                categories.add(key)
            } else if (!value && key in categories) {
                categories.remove(key)
            }
        } catch (e: Exception) {
            logError(Category.SETTINGS, "Wrong settings key: $logCategory")
        }
    }

    @JvmStatic
    fun isLoggable(level: Level, logCategory: Category): Boolean {
        return level.logLevel <= logLevel && logCategory in categories
    }

    @JvmStatic
    fun logMessage(
        logLevel: Level,
        logCategory: Category,
        logMessage: String,
        e: Throwable? = null
    ) {
        if (!isLoggable(logLevel, logCategory)) return

        val message = "[$logCategory] $logMessage"
        when (logLevel) {
            Level.ERROR -> Log.e(TAG, message, e)
            Level.WARNING -> Log.w(TAG, message)
            Level.INFO -> Log.i(TAG, message)
            Level.DEBUG -> Log.d(TAG, message)
            Level.VERBOSE -> Log.v(TAG, message)
        }
    }

    @JvmStatic
    fun logException(logCategory: Category, logMessage: String, e: Throwable?) {
        logMessage(Level.ERROR, logCategory, logMessage, e)
    }

    @JvmStatic
    fun logError(logCategory: Category, logMessage: String) {
        logMessage(Level.ERROR, logCategory, logMessage)
    }

    @JvmStatic
    fun logWarning(logCategory: Category, logMessage: String) {
        logMessage(Level.WARNING, logCategory, logMessage)
    }

    @JvmStatic
    fun logInfo(logCategory: Category, logMessage: String) {
        logMessage(Level.INFO, logCategory, logMessage)
    }

    @JvmStatic
    fun logDebug(logCategory: Category, logMessage: String) {
        logMessage(Level.DEBUG, logCategory, logMessage)
    }

    @JvmStatic
    fun logVerbose(logCategory: Category, logMessage: String) {
        logMessage(Level.VERBOSE, logCategory, logMessage)
    }

    enum class Level(val logLevel: Int) {
        ERROR(1),
        WARNING(2),
        INFO(3),
        DEBUG(4),
        VERBOSE(5)
    }

    enum class Category {
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
