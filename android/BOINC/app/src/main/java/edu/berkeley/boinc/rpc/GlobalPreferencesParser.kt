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
package edu.berkeley.boinc.rpc

import android.util.Xml
import edu.berkeley.boinc.utils.Logging
import org.xml.sax.Attributes
import org.xml.sax.SAXException

class GlobalPreferencesParser : BaseParser() {
    lateinit var globalPreferences: GlobalPreferences
        private set
    private var mInsideDayPrefs = false
    private var mDayOfWeek = 0
    private var mTempCpuTimeSpan: TimeSpan? = null
    private var mTempNetTimeSpan: TimeSpan? = null

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        when {
            localName.equals(GLOBAL_PREFERENCES_TAG, ignoreCase = true) -> {
                globalPreferences = GlobalPreferences()
            }
            localName.equals(DAY_PREFS_TAG, ignoreCase = true) -> {
                mInsideDayPrefs = true
            }
            else -> { // Another element, hopefully primitive and not constructor
                // (although unknown constructor does not hurt, because there will be primitive start anyway)
                mElementStarted = true
                mCurrentElement.setLength(0)
            }
        }
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            if (!localName.equals(GLOBAL_PREFERENCES_TAG, ignoreCase = true) &&
                    localName.equals(DAY_PREFS_TAG, ignoreCase = true)) { // closing <day_prefs>
                if (mDayOfWeek in 0..6) {
                    globalPreferences.cpuTimes.weekPrefs[mDayOfWeek] = mTempCpuTimeSpan
                    globalPreferences.netTimes.weekPrefs[mDayOfWeek] = mTempNetTimeSpan
                }
                mTempCpuTimeSpan = null
                mTempNetTimeSpan = null
                mInsideDayPrefs = false
            } else if (mInsideDayPrefs) {
                trimEnd()
                when {
                    localName.equals(DAY_OF_WEEK_TAG, ignoreCase = true) -> {
                        mDayOfWeek = mCurrentElement.toInt()
                    }
                    localName.equals(TimePreferences.Fields.START_HOUR, ignoreCase = true) -> {
                        if (mTempCpuTimeSpan == null) {
                            mTempCpuTimeSpan = TimeSpan()
                        }
                        mTempCpuTimeSpan!!.startHour = mCurrentElement.toDouble()
                    }
                    localName.equals(TimePreferences.Fields.END_HOUR, ignoreCase = true) -> {
                        if (mTempCpuTimeSpan == null) {
                            mTempCpuTimeSpan = TimeSpan()
                        }
                        mTempCpuTimeSpan!!.endHour = mCurrentElement.toDouble()
                    }
                    localName.equals(NET_START_HOUR_TAG, ignoreCase = true) -> {
                        if (mTempNetTimeSpan == null) {
                            mTempNetTimeSpan = TimeSpan()
                        }
                        mTempNetTimeSpan!!.startHour = mCurrentElement.toDouble()
                    }
                    localName.equals(NET_END_HOUR_TAG, ignoreCase = true) -> {
                        if (mTempNetTimeSpan == null) {
                            mTempNetTimeSpan = TimeSpan()
                        }
                        mTempNetTimeSpan!!.endHour = mCurrentElement.toDouble()
                    }
                }
            } else { // Not the closing tag - we decode possible inner tags
                mCurrentElement.trimEnd()
                when {
                    localName.equals(GlobalPreferences.Fields.RUN_ON_BATTERIES, ignoreCase = true) -> {
                        globalPreferences.runOnBatteryPower = mCurrentElement.toInt() != 0
                    }
                    localName.equals(GlobalPreferences.Fields.BATTERY_CHARGE_MIN_PCT, ignoreCase = true) -> {
                        globalPreferences.batteryChargeMinPct = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.BATTERY_MAX_TEMPERATURE, ignoreCase = true) -> {
                        globalPreferences.batteryMaxTemperature = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.RUN_GPU_IF_USER_ACTIVE, ignoreCase = true) -> {
                        globalPreferences.runGpuIfUserActive = mCurrentElement.toInt() != 0
                    }
                    localName.equals(GlobalPreferences.Fields.RUN_IF_USER_ACTIVE, ignoreCase = true) -> {
                        globalPreferences.runIfUserActive = mCurrentElement.toInt() != 0
                    }
                    localName.equals(GlobalPreferences.Fields.IDLE_TIME_TO_RUN, ignoreCase = true) -> {
                        globalPreferences.idleTimeToRun = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.SUSPEND_CPU_USAGE, ignoreCase = true) -> {
                        globalPreferences.suspendCpuUsage = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.LEAVE_APPS_IN_MEMORY, ignoreCase = true) -> {
                        globalPreferences.leaveAppsInMemory = mCurrentElement.toInt() != 0
                    }
                    localName.equals(GlobalPreferences.Fields.DONT_VERIFY_IMAGES, ignoreCase = true) -> {
                        globalPreferences.doNotVerifyImages = mCurrentElement.toInt() != 0
                    }
                    localName.equals(GlobalPreferences.Fields.WORK_BUF_MIN_DAYS, ignoreCase = true) -> {
                        globalPreferences.workBufMinDays = 0.00001.coerceAtLeast(mCurrentElement.toDouble())
                    }
                    localName.equals(GlobalPreferences.Fields.WORK_BUF_ADDITIONAL_DAYS, ignoreCase = true) -> {
                        globalPreferences.workBufAdditionalDays = 0.0.coerceAtLeast(mCurrentElement.toDouble())
                    }
                    localName.equals(GlobalPreferences.Fields.MAX_NCPUS_PCT, ignoreCase = true) -> {
                        globalPreferences.maxNoOfCPUsPct = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.CPU_SCHEDULING_PERIOD_MINUTES, ignoreCase = true) -> {
                        var value = mCurrentElement.toDouble()
                        if (value < 0.00001) {
                            value = 60.0
                        }
                        globalPreferences.cpuSchedulingPeriodMinutes = value
                    }
                    localName.equals(GlobalPreferences.Fields.DISK_INTERVAL, ignoreCase = true) -> {
                        globalPreferences.diskInterval = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.DISK_MAX_USED_GB, ignoreCase = true) -> {
                        globalPreferences.diskMaxUsedGB = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.DISK_MAX_USED_PCT, ignoreCase = true) -> {
                        globalPreferences.diskMaxUsedPct = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.DISK_MIN_FREE_GB, ignoreCase = true) -> {
                        globalPreferences.diskMinFreeGB = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.RAM_MAX_USED_BUSY_FRAC, ignoreCase = true) -> {
                        globalPreferences.ramMaxUsedBusyFrac = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.RAM_MAX_USED_IDLE_FRAC, ignoreCase = true) -> {
                        globalPreferences.ramMaxUsedIdleFrac = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.MAX_BYTES_SEC_UP, ignoreCase = true) -> {
                        globalPreferences.maxBytesSecUp = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.MAX_BYTES_SEC_DOWN, ignoreCase = true) -> {
                        globalPreferences.maxBytesSecDown = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.CPU_USAGE_LIMIT, ignoreCase = true) -> {
                        globalPreferences.cpuUsageLimit = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.DAILY_XFER_PERIOD_MB, ignoreCase = true) -> {
                        globalPreferences.dailyTransferLimitMB = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.DAILY_XFER_PERIOD_DAYS, ignoreCase = true) -> {
                        globalPreferences.dailyTransferPeriodDays = mCurrentElement.toInt()
                    }
                    localName.equals(TimePreferences.Fields.START_HOUR, ignoreCase = true) -> {
                        globalPreferences.cpuTimes.startHour = mCurrentElement.toDouble()
                    }
                    localName.equals(TimePreferences.Fields.END_HOUR, ignoreCase = true) -> {
                        globalPreferences.cpuTimes.endHour = mCurrentElement.toDouble()
                    }
                    localName.equals(NET_START_HOUR_TAG, ignoreCase = true) -> {
                        globalPreferences.netTimes.startHour = mCurrentElement.toDouble()
                    }
                    localName.equals(NET_END_HOUR_TAG, ignoreCase = true) -> {
                        globalPreferences.netTimes.endHour = mCurrentElement.toDouble()
                    }
                    localName.equals(GlobalPreferences.Fields.OVERRIDE_FILE_PRESENT, ignoreCase = true) -> {
                        globalPreferences.overrideFilePresent = mCurrentElement.toInt() != 0
                    }
                    localName.equals(GlobalPreferences.Fields.NETWORK_WIFI_ONLY, ignoreCase = true) -> {
                        globalPreferences.networkWiFiOnly = mCurrentElement.toInt() != 0
                    }
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "GlobalPreferencesParser.endElement error: ", e)
        }
        mElementStarted = false
    }

    companion object {
        const val GLOBAL_PREFERENCES_TAG = "global_preferences"
        const val DAY_PREFS_TAG = "day_prefs"
        const val DAY_OF_WEEK_TAG = "day_of_week"
        const val NET_START_HOUR_TAG = "net_start_hour"
        const val NET_END_HOUR_TAG = "net_end_hour"
        @JvmStatic
        fun parse(rpcResult: String?): GlobalPreferences? {
            return try {
                val parser = GlobalPreferencesParser()
                Xml.parse(rpcResult, parser)
                parser.globalPreferences
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "GlobalPreferencesParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "GlobalPreferencesParser: $rpcResult")

                null
            }
        }
    }
}
