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
package edu.berkeley.boinc.rpc.serialization

import com.fasterxml.jackson.core.JsonGenerator
import com.fasterxml.jackson.core.JsonProcessingException
import com.fasterxml.jackson.databind.SerializerProvider
import com.fasterxml.jackson.databind.ser.std.StdSerializer
import edu.berkeley.boinc.rpc.GlobalPreferences
import edu.berkeley.boinc.rpc.GlobalPreferencesParser
import edu.berkeley.boinc.rpc.TimePreferences
import edu.berkeley.boinc.rpc.TimeSpan
import java.io.IOException

private val defaultTimeSpanArray = arrayOfNulls<TimeSpan>(7)

private fun Boolean.toInt() = if (this) 1 else 0

class GlobalPreferencesSerializer(clazz: Class<GlobalPreferences>? = null) : StdSerializer<GlobalPreferences>(clazz) {
    @Throws(IOException::class, JsonProcessingException::class)
    override fun serialize(value: GlobalPreferences, gen: JsonGenerator, provider: SerializerProvider) {
        gen.writeStartObject()
        gen.writeNumberField(GlobalPreferences.Fields.RUN_ON_BATTERIES, value.runOnBatteryPower.toInt())
        gen.writeNumberField(GlobalPreferences.Fields.BATTERY_CHARGE_MIN_PCT, value.batteryChargeMinPct)
        gen.writeNumberField(GlobalPreferences.Fields.BATTERY_MAX_TEMPERATURE, value.batteryMaxTemperature)
        gen.writeNumberField(GlobalPreferences.Fields.RUN_GPU_IF_USER_ACTIVE, value.runGpuIfUserActive.toInt())
        gen.writeNumberField(GlobalPreferences.Fields.RUN_IF_USER_ACTIVE, value.runIfUserActive.toInt())
        gen.writeNumberField(GlobalPreferences.Fields.IDLE_TIME_TO_RUN, value.idleTimeToRun)
        gen.writeNumberField(GlobalPreferences.Fields.SUSPEND_CPU_USAGE, value.suspendCpuUsage)
        gen.writeNumberField(TimePreferences.Fields.START_HOUR, value.cpuTimes.startHour)
        gen.writeNumberField(TimePreferences.Fields.END_HOUR, value.cpuTimes.endHour)
        gen.writeNumberField(GlobalPreferencesParser.NET_START_HOUR_TAG, value.netTimes.startHour)
        gen.writeNumberField(GlobalPreferencesParser.NET_END_HOUR_TAG, value.netTimes.endHour)
        gen.writeNumberField(GlobalPreferences.Fields.MAX_NCPUS_PCT, value.maxNoOfCPUsPct)
        gen.writeNumberField(GlobalPreferences.Fields.LEAVE_APPS_IN_MEMORY, value.leaveAppsInMemory.toInt())
        gen.writeNumberField(GlobalPreferences.Fields.DONT_VERIFY_IMAGES, value.doNotVerifyImages.toInt())
        gen.writeNumberField(GlobalPreferences.Fields.WORK_BUF_MIN_DAYS, value.workBufMinDays)
        gen.writeNumberField(GlobalPreferences.Fields.WORK_BUF_ADDITIONAL_DAYS, value.workBufAdditionalDays)
        gen.writeNumberField(GlobalPreferences.Fields.DISK_INTERVAL, value.diskInterval)
        gen.writeNumberField(GlobalPreferences.Fields.CPU_SCHEDULING_PERIOD_MINUTES, value.cpuSchedulingPeriodMinutes)
        gen.writeNumberField(GlobalPreferences.Fields.DISK_MAX_USED_GB, value.diskMaxUsedGB)
        gen.writeNumberField(GlobalPreferences.Fields.DISK_MAX_USED_PCT, value.diskMaxUsedPct)
        gen.writeNumberField(GlobalPreferences.Fields.DISK_MIN_FREE_GB, value.diskMinFreeGB)
        gen.writeNumberField("ram_max_used_busy_pct", value.ramMaxUsedBusyFrac)
        gen.writeNumberField("ram_max_used_idle_pct", value.ramMaxUsedIdleFrac)
        gen.writeNumberField(GlobalPreferences.Fields.MAX_BYTES_SEC_UP, value.maxBytesSecUp)
        gen.writeNumberField(GlobalPreferences.Fields.MAX_BYTES_SEC_DOWN, value.maxBytesSecDown)
        gen.writeNumberField(GlobalPreferences.Fields.CPU_USAGE_LIMIT, value.cpuUsageLimit)
        gen.writeNumberField(GlobalPreferences.Fields.DAILY_XFER_PERIOD_MB, value.dailyTransferLimitMB)
        gen.writeNumberField(GlobalPreferences.Fields.DAILY_XFER_PERIOD_DAYS, value.dailyTransferPeriodDays)
        gen.writeNumberField(GlobalPreferences.Fields.NETWORK_WIFI_ONLY, value.networkWiFiOnly.toInt())

        if (!value.cpuTimes.weekPrefs.contentEquals(defaultTimeSpanArray)) {
            val cpuWeekPrefs = value.cpuTimes.weekPrefs
            for (i in cpuWeekPrefs.indices) {
                if (cpuWeekPrefs[i] == null) continue
                gen.writeObjectFieldStart(GlobalPreferencesParser.DAY_PREFS_TAG)
                gen.writeNumberField(GlobalPreferencesParser.DAY_OF_WEEK_TAG, i)
                gen.writeNumberField(TimePreferences.Fields.START_HOUR, cpuWeekPrefs[i]!!.startHour)
                gen.writeNumberField(TimePreferences.Fields.END_HOUR, cpuWeekPrefs[i]!!.endHour)
                gen.writeEndObject()
            }
        }

        if (!value.netTimes.weekPrefs.contentEquals(defaultTimeSpanArray)) {
            val netWeekPrefs = value.netTimes.weekPrefs
            for (i in netWeekPrefs.indices) {
                if (netWeekPrefs[i] == null) continue
                gen.writeObjectFieldStart(GlobalPreferencesParser.DAY_PREFS_TAG)
                gen.writeNumberField(GlobalPreferencesParser.DAY_OF_WEEK_TAG, i)
                gen.writeNumberField(GlobalPreferencesParser.NET_START_HOUR_TAG, netWeekPrefs[i]!!.startHour)
                gen.writeNumberField(GlobalPreferencesParser.NET_END_HOUR_TAG, netWeekPrefs[i]!!.endHour)
                gen.writeEndObject()
            }
        }

        gen.writeEndObject()
    }
}
