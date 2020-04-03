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
package edu.berkeley.boinc.rpc;

import android.util.Xml;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.xml.sax.SAXException;

import kotlin.UninitializedPropertyAccessException;

import static org.junit.Assert.*;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class GlobalPreferencesParserTest {
    private GlobalPreferencesParser globalPreferencesParser;
    private GlobalPreferences expected;

    @Before
    public void setUp() {
        globalPreferencesParser = new GlobalPreferencesParser();
        expected = new GlobalPreferences();
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParse_whenRpcStringIsNull_thenExpectUninitializedPropertyAccessException() {
        mockStatic(Xml.class);

        GlobalPreferencesParser.parse(null);
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParse_whenRpcStringIsEmpty_thenExpectUninitializedPropertyAccessException() {
        mockStatic(Xml.class);

        GlobalPreferencesParser.parse("");
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectElementStarted() throws SAXException {
        globalPreferencesParser.startElement(null, "", null, null);

        assertTrue(globalPreferencesParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsGlobalPreferences_thenExpectElementNotStarted()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        assertFalse(globalPreferencesParser.mElementStarted);
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParser_whenLocalNameIsEmpty_thenExpectUninitializedPropertyAccessException() throws SAXException {
        globalPreferencesParser.startElement(null, "", null, null);

        globalPreferencesParser.getGlobalPreferences();
    }

    @Test
    public void testParser_whenLocalNameIsGlobalPreferencesTag_thenExpectDefaultGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRunOnBatteries0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.RUN_ON_BATTERIES,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.RUN_ON_BATTERIES,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setRunOnBatteryPower(false);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRunOnBatteries1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.RUN_ON_BATTERIES,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.RUN_ON_BATTERIES,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setRunOnBatteryPower(true);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasInvalidBatteryChargeMinPct_thenExpectDefaultGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.BATTERY_CHARGE_MIN_PCT,
                                             null, null);
        globalPreferencesParser.characters("Fifty".toCharArray(), 0, 5);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.BATTERY_CHARGE_MIN_PCT,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        assertEquals(new GlobalPreferences(), globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasBatteryChargeMinPct_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.BATTERY_CHARGE_MIN_PCT,
                                             null, null);
        globalPreferencesParser.characters("50".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.BATTERY_CHARGE_MIN_PCT,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setBatteryChargeMinPct(50.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasBatteryMaxTemperature_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.BATTERY_MAX_TEMPERATURE,
                                             null, null);
        globalPreferencesParser.characters("35".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.BATTERY_MAX_TEMPERATURE,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setBatteryMaxTemperature(35.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRunGpuIfUserActive0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.RUN_GPU_IF_USER_ACTIVE,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.RUN_GPU_IF_USER_ACTIVE,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setRunGpuIfUserActive(false);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRunGpuIfUserActive1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.RUN_GPU_IF_USER_ACTIVE,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.RUN_GPU_IF_USER_ACTIVE,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setRunGpuIfUserActive(true);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRunIfUserActive0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.RUN_IF_USER_ACTIVE,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.RUN_IF_USER_ACTIVE,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setRunIfUserActive(false);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRunIfUserActive1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.RUN_IF_USER_ACTIVE,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.RUN_IF_USER_ACTIVE,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setRunIfUserActive(true);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasIdleTimeToRun_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.IDLE_TIME_TO_RUN,
                                             null, null);
        globalPreferencesParser.characters("10".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.IDLE_TIME_TO_RUN,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setIdleTimeToRun(10.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasSuspendCpuUsage_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.SUSPEND_CPU_USAGE,
                                             null, null);
        globalPreferencesParser.characters("60".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.SUSPEND_CPU_USAGE,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setSuspendCpuUsage(60.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasLeaveAppsInMemory0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.LEAVE_APPS_IN_MEMORY,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.LEAVE_APPS_IN_MEMORY,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setLeaveAppsInMemory(false);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasLeaveAppsInMemory1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.LEAVE_APPS_IN_MEMORY,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.LEAVE_APPS_IN_MEMORY,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setLeaveAppsInMemory(true);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDontVerifyImages0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.DONT_VERIFY_IMAGES,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.DONT_VERIFY_IMAGES,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setDoNotVerifyImages(false);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDontVerifyImages1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.DONT_VERIFY_IMAGES,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.DONT_VERIFY_IMAGES,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setDoNotVerifyImages(true);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasWorkBufMinDays0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.WORK_BUF_MIN_DAYS,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.WORK_BUF_MIN_DAYS,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setWorkBufMinDays(0.00001);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasWorkBufMinDays1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.WORK_BUF_MIN_DAYS,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.WORK_BUF_MIN_DAYS,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setWorkBufMinDays(1.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasWorkBufAdditionalDaysMinus1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.WORK_BUF_ADDITIONAL_DAYS,
                                             null, null);
        globalPreferencesParser.characters("-1".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.WORK_BUF_ADDITIONAL_DAYS,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setWorkBufAdditionalDays(0.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasWorkBufAdditionalDays1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.WORK_BUF_ADDITIONAL_DAYS,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.WORK_BUF_ADDITIONAL_DAYS,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setWorkBufAdditionalDays(1.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasMaxNCpusPct_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.MAX_NCPUS_PCT,
                                             null, null);
        globalPreferencesParser.characters("0.75".toCharArray(), 0, 4);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.MAX_NCPUS_PCT,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setMaxNoOfCPUsPct(0.75);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasCpuSchedulingPeriodMinutes0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.CPU_SCHEDULING_PERIOD_MINUTES,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.CPU_SCHEDULING_PERIOD_MINUTES,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setCpuSchedulingPeriodMinutes(60.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasCpuSchedulingPeriodMinutes1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.CPU_SCHEDULING_PERIOD_MINUTES,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.CPU_SCHEDULING_PERIOD_MINUTES,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setCpuSchedulingPeriodMinutes(1.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDiskInterval_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.DISK_INTERVAL,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.DISK_INTERVAL,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setDiskInterval(1.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDiskMaxUsedGb_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.DISK_MAX_USED_GB,
                                             null, null);
        globalPreferencesParser.characters("10".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.DISK_MAX_USED_GB,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setDiskMaxUsedGB(10.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDiskMaxUsedPct_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.DISK_MAX_USED_PCT,
                                             null, null);
        globalPreferencesParser.characters("10".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.DISK_MAX_USED_PCT,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setDiskMaxUsedPct(10.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDiskMinFreeGb_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.DISK_MIN_FREE_GB,
                                             null, null);
        globalPreferencesParser.characters("5".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.DISK_MIN_FREE_GB,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setDiskMinFreeGB(5.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRamMaxUsedBusyFrac_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.RAM_MAX_USED_BUSY_FRAC,
                                             null, null);
        globalPreferencesParser.characters("0.5".toCharArray(), 0, 3);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.RAM_MAX_USED_BUSY_FRAC,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setRamMaxUsedBusyFrac(0.5);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRamMaxUsedIdleFrac_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.RAM_MAX_USED_IDLE_FRAC,
                                             null, null);
        globalPreferencesParser.characters("0.5".toCharArray(), 0, 3);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.RAM_MAX_USED_IDLE_FRAC,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setRamMaxUsedIdleFrac(0.5);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasMaxBytesSecUp_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.MAX_BYTES_SEC_UP,
                                             null, null);
        globalPreferencesParser.characters("0.5".toCharArray(), 0, 3);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.MAX_BYTES_SEC_UP,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setMaxBytesSecUp(0.5);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasMaxBytesSecDown_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.MAX_BYTES_SEC_DOWN,
                                             null, null);
        globalPreferencesParser.characters("0.5".toCharArray(), 0, 3);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.MAX_BYTES_SEC_DOWN,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setMaxBytesSecDown(0.5);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasCpuUsageLimit_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.CPU_USAGE_LIMIT,
                                             null, null);
        globalPreferencesParser.characters("60".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.CPU_USAGE_LIMIT,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setCpuUsageLimit(60.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDailyTransferLimitMb_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.DAILY_XFER_PERIOD_MB,
                                             null, null);
        globalPreferencesParser.characters("250".toCharArray(), 0, 3);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.DAILY_XFER_PERIOD_MB,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setDailyTransferLimitMB(250.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDailyTransferPeriodDays_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.DAILY_XFER_PERIOD_DAYS,
                                             null, null);
        globalPreferencesParser.characters("2".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.DAILY_XFER_PERIOD_DAYS,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setDailyTransferPeriodDays(2);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasStartHour_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, TimePreferences.Fields.START_HOUR,
                                             null, null);
        globalPreferencesParser.characters("7".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, TimePreferences.Fields.START_HOUR,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.getCpuTimes().setStartHour(7.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasEndHour_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, TimePreferences.Fields.END_HOUR,
                                             null, null);
        globalPreferencesParser.characters("18".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, TimePreferences.Fields.END_HOUR,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.getCpuTimes().setEndHour(18.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasNetStartHour_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.NET_START_HOUR_TAG,
                                             null, null);
        globalPreferencesParser.characters("7".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.NET_START_HOUR_TAG,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.getNetTimes().setStartHour(7.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasNetEndHour_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.NET_END_HOUR_TAG,
                                             null, null);
        globalPreferencesParser.characters("18".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.NET_END_HOUR_TAG,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.getNetTimes().setEndHour(18.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasOverrideFilePresent0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.OVERRIDE_FILE_PRESENT,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.OVERRIDE_FILE_PRESENT,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setOverrideFilePresent(false);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasOverrideFilePresent1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.OVERRIDE_FILE_PRESENT,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.OVERRIDE_FILE_PRESENT,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setOverrideFilePresent(true);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasNetworkWiFiOnly0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.NETWORK_WIFI_ONLY,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.NETWORK_WIFI_ONLY,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setNetworkWiFiOnly(false);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasNetworkWiFiOnly1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.NETWORK_WIFI_ONLY,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.NETWORK_WIFI_ONLY,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.setNetworkWiFiOnly(true);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDayPrefsWithDayOfWeekAndStartHour_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.DAY_PREFS_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
                                           null);
        globalPreferencesParser.startElement(null, TimePreferences.Fields.START_HOUR,
                                             null, null);
        globalPreferencesParser.characters("7".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, TimePreferences.Fields.START_HOUR,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.DAY_PREFS_TAG,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        final TimeSpan expectedTimeSpan = new TimeSpan();
        expectedTimeSpan.setStartHour(7.0);
        expected.getCpuTimes().getWeekPrefs()[1] = expectedTimeSpan;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDayPrefsWithDayOfWeekAndEndHour_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.DAY_PREFS_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
                                           null);
        globalPreferencesParser.startElement(null, TimePreferences.Fields.END_HOUR,
                                             null, null);
        globalPreferencesParser.characters("18".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, TimePreferences.Fields.END_HOUR,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.DAY_PREFS_TAG,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        final TimeSpan expectedTimeSpan = new TimeSpan();
        expectedTimeSpan.setEndHour(18.0);
        expected.getCpuTimes().getWeekPrefs()[1] = expectedTimeSpan;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDayPrefsWithDayOfWeekAndNetStartHour_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.DAY_PREFS_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
                                             null, null);
        globalPreferencesParser.characters("2".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
                                           null);
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.NET_START_HOUR_TAG,
                                             null, null);
        globalPreferencesParser.characters("7".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.NET_START_HOUR_TAG,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.DAY_PREFS_TAG,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        final TimeSpan expectedTimeSpan = new TimeSpan();
        expectedTimeSpan.setStartHour(7.0);
        expected.getNetTimes().getWeekPrefs()[2] = expectedTimeSpan;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDayPrefsWithDayOfWeekAndNetEndHour_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.DAY_PREFS_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
                                             null, null);
        globalPreferencesParser.characters("2".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.DAY_OF_WEEK_TAG,
                                           null);
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.NET_END_HOUR_TAG,
                                             null, null);
        globalPreferencesParser.characters("18".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.NET_END_HOUR_TAG,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.DAY_PREFS_TAG,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        final TimeSpan expectedTimeSpan = new TimeSpan();
        expectedTimeSpan.setEndHour(18.0);
        expected.getNetTimes().getWeekPrefs()[2] = expectedTimeSpan;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }
}
