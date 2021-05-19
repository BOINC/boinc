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

import android.util.Log
import android.util.Xml
import edu.berkeley.boinc.rpc.GlobalPreferencesParser.Companion.parse
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.ArgumentMatchers
import org.powermock.api.mockito.PowerMockito
import org.powermock.core.classloader.annotations.PrepareForTest
import org.powermock.modules.junit4.PowerMockRunner
import org.xml.sax.ContentHandler
import org.xml.sax.SAXException

@RunWith(PowerMockRunner::class)
@PrepareForTest(Log::class, Xml::class)
class GlobalPreferencesParserTest {
    private lateinit var globalPreferencesParser: GlobalPreferencesParser
    private lateinit var expected: GlobalPreferences
    @Before
    fun setUp() {
        globalPreferencesParser = GlobalPreferencesParser()
        expected = GlobalPreferences()
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    fun `When Rpc string is null then expect UninitializedPropertyAccessException`() {
        PowerMockito.mockStatic(Xml::class.java)
        parse(null)
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    fun `When Rpc string is empty then expect UninitializedPropertyAccessException`() {
        PowerMockito.mockStatic(Xml::class.java)
        parse("")
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown then expect null`() {
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertNull(parse(""))
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect element started`() {
        globalPreferencesParser.startElement(null, "", null, null)
        Assert.assertTrue(globalPreferencesParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is global_preferences then expect element not started`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        Assert.assertFalse(globalPreferencesParser.mElementStarted)
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect UninitializedPropertyAccessException`() {
        globalPreferencesParser.startElement(null, "", null, null)
        globalPreferencesParser.globalPreferences
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is global_preferences tag then expect default GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has run_on_batteries 0 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.RUN_ON_BATTERIES,
            null, null
        )
        globalPreferencesParser.characters("0".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.RUN_ON_BATTERIES,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.runOnBatteryPower = false
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has run_on_batteries 1 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.RUN_ON_BATTERIES,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.RUN_ON_BATTERIES,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.runOnBatteryPower = true
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has invalid battery_charge_min_pct then expect default GlobalPreferences`() {
        PowerMockito.mockStatic(Log::class.java)
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.BATTERY_CHARGE_MIN_PCT,
            null, null
        )
        globalPreferencesParser.characters("Fifty".toCharArray(), 0, 5)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.BATTERY_CHARGE_MIN_PCT,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        Assert.assertEquals(GlobalPreferences(), globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has battery_charge_min_pct then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.BATTERY_CHARGE_MIN_PCT,
            null, null
        )
        globalPreferencesParser.characters("50".toCharArray(), 0, 2)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.BATTERY_CHARGE_MIN_PCT,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.batteryChargeMinPct = 50.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has battery_max_temperature then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.BATTERY_MAX_TEMPERATURE,
            null, null
        )
        globalPreferencesParser.characters("35".toCharArray(), 0, 2)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.BATTERY_MAX_TEMPERATURE,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.batteryMaxTemperature = 35.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has run_gpu_if_user_active 0 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.RUN_GPU_IF_USER_ACTIVE,
            null, null
        )
        globalPreferencesParser.characters("0".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.RUN_GPU_IF_USER_ACTIVE,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.runGpuIfUserActive = false
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has run_gpu_if_user_active 1 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.RUN_GPU_IF_USER_ACTIVE,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.RUN_GPU_IF_USER_ACTIVE,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.runGpuIfUserActive = true
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has run_if_user_active 0 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.RUN_IF_USER_ACTIVE,
            null, null
        )
        globalPreferencesParser.characters("0".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.RUN_IF_USER_ACTIVE,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.runIfUserActive = false
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has run_if_user_active 1 then expect mMatching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.RUN_IF_USER_ACTIVE,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.RUN_IF_USER_ACTIVE,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.runIfUserActive = true
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has idle_time_to_run then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.IDLE_TIME_TO_RUN,
            null, null
        )
        globalPreferencesParser.characters("10".toCharArray(), 0, 2)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.IDLE_TIME_TO_RUN,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.idleTimeToRun = 10.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has suspend_cpu_usage then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.SUSPEND_CPU_USAGE,
            null, null
        )
        globalPreferencesParser.characters("60".toCharArray(), 0, 2)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.SUSPEND_CPU_USAGE,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.suspendCpuUsage = 60.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has leave_apps_in_memory 0 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.LEAVE_APPS_IN_MEMORY,
            null, null
        )
        globalPreferencesParser.characters("0".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.LEAVE_APPS_IN_MEMORY,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.leaveAppsInMemory = false
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has leave_apps_in_memory 1 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.LEAVE_APPS_IN_MEMORY,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.LEAVE_APPS_IN_MEMORY,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.leaveAppsInMemory = true
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has dont_verify_images 0 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.DONT_VERIFY_IMAGES,
            null, null
        )
        globalPreferencesParser.characters("0".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.DONT_VERIFY_IMAGES,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.doNotVerifyImages = false
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has dont_verify_images 1 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.DONT_VERIFY_IMAGES,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.DONT_VERIFY_IMAGES,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.doNotVerifyImages = true
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has work_buf_min_days 0 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.WORK_BUF_MIN_DAYS,
            null, null
        )
        globalPreferencesParser.characters("0".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.WORK_BUF_MIN_DAYS,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.workBufMinDays = 0.00001
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has work_buf_min_days 1 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.WORK_BUF_MIN_DAYS,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.WORK_BUF_MIN_DAYS,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.workBufMinDays = 1.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has work_buf_additional_days_minus 1 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.WORK_BUF_ADDITIONAL_DAYS,
            null, null
        )
        globalPreferencesParser.characters("-1".toCharArray(), 0, 2)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.WORK_BUF_ADDITIONAL_DAYS,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.workBufAdditionalDays = 0.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has work_buf_additional_days 1 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.WORK_BUF_ADDITIONAL_DAYS,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.WORK_BUF_ADDITIONAL_DAYS,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.workBufAdditionalDays = 1.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has max_ncpus_pct then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.MAX_NCPUS_PCT,
            null, null
        )
        globalPreferencesParser.characters("0.75".toCharArray(), 0, 4)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.MAX_NCPUS_PCT,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.maxNoOfCPUsPct = 0.75
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has cpu_scheduling_period_minutes 0 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.CPU_SCHEDULING_PERIOD_MINUTES,
            null, null
        )
        globalPreferencesParser.characters("0".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.CPU_SCHEDULING_PERIOD_MINUTES,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.cpuSchedulingPeriodMinutes = 60.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has cpu_scheduling_period_minutes 1 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.CPU_SCHEDULING_PERIOD_MINUTES,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.CPU_SCHEDULING_PERIOD_MINUTES,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.cpuSchedulingPeriodMinutes = 1.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has disk_interval then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.DISK_INTERVAL,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.DISK_INTERVAL,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.diskInterval = 1.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has disk_max_used_gb then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.DISK_MAX_USED_GB,
            null, null
        )
        globalPreferencesParser.characters("10".toCharArray(), 0, 2)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.DISK_MAX_USED_GB,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.diskMaxUsedGB = 10.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has disk_max_used_pct then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.DISK_MAX_USED_PCT,
            null, null
        )
        globalPreferencesParser.characters("10".toCharArray(), 0, 2)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.DISK_MAX_USED_PCT,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.diskMaxUsedPct = 10.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has disk_min_free_gb then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.DISK_MIN_FREE_GB,
            null, null
        )
        globalPreferencesParser.characters("5".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.DISK_MIN_FREE_GB,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.diskMinFreeGB = 5.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has ram_max_used_busy_frac then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.RAM_MAX_USED_BUSY_FRAC,
            null, null
        )
        globalPreferencesParser.characters("0.5".toCharArray(), 0, 3)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.RAM_MAX_USED_BUSY_FRAC,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.ramMaxUsedBusyFrac = 0.5
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has ram_max_used_idle_frac then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.RAM_MAX_USED_IDLE_FRAC,
            null, null
        )
        globalPreferencesParser.characters("0.5".toCharArray(), 0, 3)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.RAM_MAX_USED_IDLE_FRAC,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.ramMaxUsedIdleFrac = 0.5
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has max_bytes_sec_up then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.MAX_BYTES_SEC_UP,
            null, null
        )
        globalPreferencesParser.characters("0.5".toCharArray(), 0, 3)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.MAX_BYTES_SEC_UP,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.maxBytesSecUp = 0.5
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has max_bytes_sec_down then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.MAX_BYTES_SEC_DOWN,
            null, null
        )
        globalPreferencesParser.characters("0.5".toCharArray(), 0, 3)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.MAX_BYTES_SEC_DOWN,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.maxBytesSecDown = 0.5
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has cpu_usage_limit then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.CPU_USAGE_LIMIT,
            null, null
        )
        globalPreferencesParser.characters("60".toCharArray(), 0, 2)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.CPU_USAGE_LIMIT,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.cpuUsageLimit = 60.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has daily_transfer_limit_mb then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.DAILY_XFER_PERIOD_MB,
            null, null
        )
        globalPreferencesParser.characters("250".toCharArray(), 0, 3)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.DAILY_XFER_PERIOD_MB,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.dailyTransferLimitMB = 250.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has daily_transfer_period_days then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.DAILY_XFER_PERIOD_DAYS,
            null, null
        )
        globalPreferencesParser.characters("2".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.DAILY_XFER_PERIOD_DAYS,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.dailyTransferPeriodDays = 2
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has start_hour then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, TimePreferences.Fields.START_HOUR,
            null, null
        )
        globalPreferencesParser.characters("7".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, TimePreferences.Fields.START_HOUR,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.cpuTimes.startHour = 7.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has end_hour then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, TimePreferences.Fields.END_HOUR,
            null, null
        )
        globalPreferencesParser.characters("18".toCharArray(), 0, 2)
        globalPreferencesParser.endElement(
            null, TimePreferences.Fields.END_HOUR,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.cpuTimes.endHour = 18.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has net_start_hour then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.NET_START_HOUR_TAG,
            null, null
        )
        globalPreferencesParser.characters("7".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.NET_START_HOUR_TAG,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.netTimes.startHour = 7.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has net_end_hour then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.NET_END_HOUR_TAG,
            null, null
        )
        globalPreferencesParser.characters("18".toCharArray(), 0, 2)
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.NET_END_HOUR_TAG,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.netTimes.endHour = 18.0
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has override_file_present 0 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.OVERRIDE_FILE_PRESENT,
            null, null
        )
        globalPreferencesParser.characters("0".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.OVERRIDE_FILE_PRESENT,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.overrideFilePresent = false
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has override_file_present 1 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.OVERRIDE_FILE_PRESENT,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.OVERRIDE_FILE_PRESENT,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.overrideFilePresent = true
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has network_wifi_only 0 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.NETWORK_WIFI_ONLY,
            null, null
        )
        globalPreferencesParser.characters("0".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.NETWORK_WIFI_ONLY,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.networkWiFiOnly = false
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has network_wifi_only 1 then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferences.Fields.NETWORK_WIFI_ONLY,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferences.Fields.NETWORK_WIFI_ONLY,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        expected.networkWiFiOnly = true
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has day_prefs with day_of_week and start_hour then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.DAY_PREFS_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
            null
        )
        globalPreferencesParser.startElement(
            null, TimePreferences.Fields.START_HOUR,
            null, null
        )
        globalPreferencesParser.characters("7".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, TimePreferences.Fields.START_HOUR,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.DAY_PREFS_TAG,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        val expectedTimeSpan = TimeSpan()
        expectedTimeSpan.startHour = 7.0
        expected.cpuTimes.weekPrefs[1] = expectedTimeSpan
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has day_prefs with day_of_week and end_hour then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.DAY_PREFS_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
            null, null
        )
        globalPreferencesParser.characters("1".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
            null
        )
        globalPreferencesParser.startElement(
            null, TimePreferences.Fields.END_HOUR,
            null, null
        )
        globalPreferencesParser.characters("18".toCharArray(), 0, 2)
        globalPreferencesParser.endElement(
            null, TimePreferences.Fields.END_HOUR,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.DAY_PREFS_TAG,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        val expectedTimeSpan = TimeSpan()
        expectedTimeSpan.endHour = 18.0
        expected.cpuTimes.weekPrefs[1] = expectedTimeSpan
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has day_prefs with day_of_week and net_start_hour then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.DAY_PREFS_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
            null, null
        )
        globalPreferencesParser.characters("2".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
            null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.NET_START_HOUR_TAG,
            null, null
        )
        globalPreferencesParser.characters("7".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.NET_START_HOUR_TAG,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.DAY_PREFS_TAG,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        val expectedTimeSpan = TimeSpan()
        expectedTimeSpan.startHour = 7.0
        expected.netTimes.weekPrefs[2] = expectedTimeSpan
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml GlobalPreferences has day_prefs with day_of_week and net_end_hour then expect matching GlobalPreferences`() {
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.DAY_PREFS_TAG,
            null, null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
            null, null
        )
        globalPreferencesParser.characters("2".toCharArray(), 0, 1)
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
            null
        )
        globalPreferencesParser.startElement(
            null, GlobalPreferencesParser.NET_END_HOUR_TAG,
            null, null
        )
        globalPreferencesParser.characters("18".toCharArray(), 0, 2)
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.NET_END_HOUR_TAG,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.DAY_PREFS_TAG,
            null
        )
        globalPreferencesParser.endElement(
            null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
            null
        )
        val expectedTimeSpan = TimeSpan()
        expectedTimeSpan.endHour = 18.0
        expected.netTimes.weekPrefs[2] = expectedTimeSpan
        Assert.assertEquals(expected, globalPreferencesParser.globalPreferences)
    }
}
