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

import android.os.Build
import android.os.Parcel
import android.os.Parcelable

data class GlobalPreferences(
        var batteryChargeMinPct: Double = 0.0,
        var batteryMaxTemperature: Double = 0.0,
        var idleTimeToRun: Double = 0.0,
        var suspendCpuUsage: Double = 0.0,
        var workBufMinDays: Double = 0.0,
        var workBufAdditionalDays: Double = 0.0,
        var maxNoOfCPUsPct: Double = 0.0,
        var cpuSchedulingPeriodMinutes: Double = 0.0,
        var diskInterval: Double = 0.0,
        var diskMaxUsedGB: Double = 0.0,
        var diskMaxUsedPct: Double = 0.0,
        var diskMinFreeGB: Double = 0.0,
        var ramMaxUsedBusyFrac: Double = 0.0,
        var ramMaxUsedIdleFrac: Double = 0.0,
        var maxBytesSecUp: Double = 0.0,
        var maxBytesSecDown: Double = 0.0,
        var cpuUsageLimit: Double = 0.0,
        var dailyTransferLimitMB: Double = 0.0,
        var dailyTransferPeriodDays: Int = 0,
        var cpuTimes: TimePreferences = TimePreferences(),
        var netTimes: TimePreferences = TimePreferences(),
        var runOnBatteryPower: Boolean = false,
        var runIfUserActive: Boolean = false,
        var runGpuIfUserActive: Boolean = false,
        var leaveAppsInMemory: Boolean = false,
        var doNotVerifyImages: Boolean = false,
        var overrideFilePresent: Boolean = false,
        var networkWiFiOnly: Boolean = false
) : Parcelable {
    private constructor(parcel: Parcel) : this(
            parcel.readDouble(), parcel.readDouble(), parcel.readDouble(), parcel.readDouble(),
            parcel.readDouble(), parcel.readDouble(), parcel.readDouble(), parcel.readDouble(),
            parcel.readDouble(), parcel.readDouble(), parcel.readDouble(), parcel.readDouble(),
            parcel.readDouble(), parcel.readDouble(), parcel.readDouble(), parcel.readDouble(),
            parcel.readDouble(), parcel.readDouble(), parcel.readInt(),
            parcel.readValue(TimePreferences::class.java.classLoader) as TimePreferences,
            parcel.readValue(TimePreferences::class.java.classLoader) as TimePreferences
    ) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            val bArray = parcel.createBooleanArray()!!
            runOnBatteryPower = bArray[0]
            runIfUserActive = bArray[1]
            runGpuIfUserActive = bArray[2]
            leaveAppsInMemory = bArray[3]
            doNotVerifyImages = bArray[4]
            overrideFilePresent = bArray[5]
            networkWiFiOnly = bArray[6]
        } else {
            runOnBatteryPower = parcel.readBoolean()
            runIfUserActive = parcel.readBoolean()
            runGpuIfUserActive = parcel.readBoolean()
            leaveAppsInMemory = parcel.readBoolean()
            doNotVerifyImages = parcel.readBoolean()
            overrideFilePresent = parcel.readBoolean()
            networkWiFiOnly = parcel.readBoolean()
        }
    }

    override fun describeContents() = 0

    override fun writeToParcel(dest: Parcel, flags: Int) {
        dest.writeDouble(batteryChargeMinPct)
        dest.writeDouble(batteryMaxTemperature)
        dest.writeDouble(idleTimeToRun)
        dest.writeDouble(suspendCpuUsage)
        dest.writeDouble(workBufMinDays)
        dest.writeDouble(workBufAdditionalDays)
        dest.writeDouble(maxNoOfCPUsPct)
        dest.writeDouble(cpuSchedulingPeriodMinutes)
        dest.writeDouble(diskInterval)
        dest.writeDouble(diskMaxUsedGB)
        dest.writeDouble(diskMaxUsedPct)
        dest.writeDouble(diskMinFreeGB)
        dest.writeDouble(ramMaxUsedBusyFrac)
        dest.writeDouble(ramMaxUsedIdleFrac)
        dest.writeDouble(maxBytesSecUp)
        dest.writeDouble(maxBytesSecDown)
        dest.writeDouble(cpuUsageLimit)
        dest.writeDouble(dailyTransferLimitMB)
        dest.writeInt(dailyTransferPeriodDays)
        dest.writeValue(cpuTimes)
        dest.writeValue(netTimes)

        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            dest.writeBooleanArray(booleanArrayOf(runOnBatteryPower, runIfUserActive, runGpuIfUserActive,
                    leaveAppsInMemory, doNotVerifyImages, overrideFilePresent, networkWiFiOnly))
        } else {
            dest.writeBoolean(runOnBatteryPower)
            dest.writeBoolean(runIfUserActive)
            dest.writeBoolean(runGpuIfUserActive)
            dest.writeBoolean(leaveAppsInMemory)
            dest.writeBoolean(doNotVerifyImages)
            dest.writeBoolean(overrideFilePresent)
            dest.writeBoolean(networkWiFiOnly)
        }
    }

    object Fields {
        const val BATTERY_CHARGE_MIN_PCT = "battery_charge_min_pct"
        const val BATTERY_MAX_TEMPERATURE = "battery_max_temperature"
        const val IDLE_TIME_TO_RUN = "idle_time_to_run"
        const val SUSPEND_CPU_USAGE = "suspend_cpu_usage"
        const val WORK_BUF_MIN_DAYS = "work_buf_min_days"
        const val WORK_BUF_ADDITIONAL_DAYS = "work_buf_additional_days"
        const val MAX_NCPUS_PCT = "max_ncpus_pct"
        const val CPU_SCHEDULING_PERIOD_MINUTES = "cpu_scheduling_period_minutes"
        const val DISK_INTERVAL = "disk_interval"
        const val DISK_MAX_USED_GB = "disk_max_used_gb"
        const val DISK_MAX_USED_PCT = "disk_max_used_pct"
        const val DISK_MIN_FREE_GB = "disk_min_free_gb"
        const val RAM_MAX_USED_BUSY_FRAC = "ram_max_used_busy_frac"
        const val RAM_MAX_USED_IDLE_FRAC = "ram_max_used_idle_frac"
        const val MAX_BYTES_SEC_UP = "max_bytes_sec_up"
        const val MAX_BYTES_SEC_DOWN = "max_bytes_sec_down"
        const val CPU_USAGE_LIMIT = "cpu_usage_limit"
        const val DAILY_XFER_PERIOD_MB = "daily_xfer_limit_mb"
        const val DAILY_XFER_PERIOD_DAYS = "daily_xfer_period_days"
        const val RUN_ON_BATTERIES = "run_on_batteries"
        const val RUN_IF_USER_ACTIVE = "run_if_user_active"
        const val RUN_GPU_IF_USER_ACTIVE = "run_gpu_if_user_active"
        const val LEAVE_APPS_IN_MEMORY = "leave_apps_in_memory"
        const val DONT_VERIFY_IMAGES = "dont_verify_images"
        const val OVERRIDE_FILE_PRESENT = "override_file_present"
        const val NETWORK_WIFI_ONLY = "network_wifi_only"
    }

    companion object {
        @JvmField
        val CREATOR: Parcelable.Creator<GlobalPreferences> = object : Parcelable.Creator<GlobalPreferences> {
            override fun createFromParcel(parcel: Parcel) = GlobalPreferences(parcel)

            override fun newArray(size: Int) = arrayOfNulls<GlobalPreferences>(size)
        }
    }
}
