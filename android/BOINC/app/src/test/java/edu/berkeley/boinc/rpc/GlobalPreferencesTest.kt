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

import com.google.common.testing.EqualsTester
import org.junit.jupiter.api.Test

class GlobalPreferencesTest {
    @Test
    fun `Test equals() and hashCode()`() {
        val timePreferences = TimePreferences(startHour = 1.0)

        EqualsTester().addEqualityGroup(GlobalPreferences(), GlobalPreferences())
                .addEqualityGroup(GlobalPreferences(batteryChargeMinPct = 1.0))
                .addEqualityGroup(GlobalPreferences(batteryMaxTemperature = 1.0))
                .addEqualityGroup(GlobalPreferences(runOnBatteryPower = true))
                .addEqualityGroup(GlobalPreferences(runIfUserActive = true))
                .addEqualityGroup(GlobalPreferences(runGpuIfUserActive = true))
                .addEqualityGroup(GlobalPreferences(idleTimeToRun = 1.0))
                .addEqualityGroup(GlobalPreferences(suspendCpuUsage = 1.0))
                .addEqualityGroup(GlobalPreferences(leaveAppsInMemory = true))
                .addEqualityGroup(GlobalPreferences(doNotVerifyImages = true))
                .addEqualityGroup(GlobalPreferences(workBufMinDays = 1.0))
                .addEqualityGroup(GlobalPreferences(workBufAdditionalDays = 1.0))
                .addEqualityGroup(GlobalPreferences(maxNoOfCPUsPct = 1.0))
                .addEqualityGroup(GlobalPreferences(cpuSchedulingPeriodMinutes = 1.0))
                .addEqualityGroup(GlobalPreferences(diskInterval = 1.0))
                .addEqualityGroup(GlobalPreferences(diskMaxUsedGB = 1.0))
                .addEqualityGroup(GlobalPreferences(diskMaxUsedPct = 1.0))
                .addEqualityGroup(GlobalPreferences(diskMinFreeGB = 1.0))
                .addEqualityGroup(GlobalPreferences(ramMaxUsedBusyFrac = 1.0))
                .addEqualityGroup(GlobalPreferences(ramMaxUsedIdleFrac = 1.0))
                .addEqualityGroup(GlobalPreferences(maxBytesSecUp = 1.0))
                .addEqualityGroup(GlobalPreferences(maxBytesSecDown = 1.0))
                .addEqualityGroup(GlobalPreferences(cpuUsageLimit = 1.0))
                .addEqualityGroup(GlobalPreferences(dailyTransferLimitMB = 1.0))
                .addEqualityGroup(GlobalPreferences(dailyTransferPeriodDays = 1))
                .addEqualityGroup(GlobalPreferences(overrideFilePresent = true))
                .addEqualityGroup(GlobalPreferences(networkWiFiOnly = true))
                .addEqualityGroup(GlobalPreferences(cpuTimes = timePreferences))
                .addEqualityGroup(GlobalPreferences(netTimes = timePreferences))
                .testEquals()
    }
}
