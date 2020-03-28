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

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectNull() {
        mockStatic(Xml.class);

        assertNull(GlobalPreferencesParser.parse(null));
    }

    @Test
    public void testParse_whenRpcStringIsEmpty_thenExpectNull() {
        mockStatic(Xml.class);

        assertNull(GlobalPreferencesParser.parse(""));
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

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectNull() throws SAXException {
        globalPreferencesParser.startElement(null, "", null, null);

        assertNull(globalPreferencesParser.getGlobalPreferences());
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
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.run_on_batteries,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.run_on_batteries,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.run_on_batteries = false;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRunOnBatteries1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.run_on_batteries,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.run_on_batteries,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.run_on_batteries = true;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasInvalidBatteryChargeMinPct_thenExpectDefaultGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.battery_charge_min_pct,
                                             null, null);
        globalPreferencesParser.characters("Fifty".toCharArray(), 0, 5);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.battery_charge_min_pct,
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
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.battery_charge_min_pct,
                                             null, null);
        globalPreferencesParser.characters("50".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.battery_charge_min_pct,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.battery_charge_min_pct = 50.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasBatteryMaxTemperature_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.battery_max_temperature,
                                             null, null);
        globalPreferencesParser.characters("35".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.battery_max_temperature,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.battery_max_temperature = 35.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRunGpuIfUserActive0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.run_gpu_if_user_active,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.run_gpu_if_user_active,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.run_gpu_if_user_active = false;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRunGpuIfUserActive1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.run_gpu_if_user_active,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.run_gpu_if_user_active,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.run_gpu_if_user_active = true;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRunIfUserActive0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.run_if_user_active,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.run_if_user_active,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.run_if_user_active = false;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRunIfUserActive1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.run_if_user_active,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.run_if_user_active,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.run_if_user_active = true;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasIdleTimeToRun_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.idle_time_to_run,
                                             null, null);
        globalPreferencesParser.characters("10".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.idle_time_to_run,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.idle_time_to_run = 10.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasSuspendCpuUsage_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.suspend_cpu_usage,
                                             null, null);
        globalPreferencesParser.characters("60".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.suspend_cpu_usage,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.suspend_cpu_usage = 60.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasLeaveAppsInMemory0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.leave_apps_in_memory,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.leave_apps_in_memory,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.leave_apps_in_memory = false;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasLeaveAppsInMemory1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.leave_apps_in_memory,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.leave_apps_in_memory,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.leave_apps_in_memory = true;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDontVerifyImages0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.dont_verify_images,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.dont_verify_images,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.dont_verify_images = false;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDontVerifyImages1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.dont_verify_images,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.dont_verify_images,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.dont_verify_images = true;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasWorkBufMinDays0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.work_buf_min_days,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.work_buf_min_days,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.work_buf_min_days = 0.00001;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasWorkBufMinDays1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.work_buf_min_days,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.work_buf_min_days,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.work_buf_min_days = 1.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasWorkBufAdditionalDaysMinus1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.work_buf_additional_days,
                                             null, null);
        globalPreferencesParser.characters("-1".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.work_buf_additional_days,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.work_buf_additional_days = 0.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasWorkBufAdditionalDays1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.work_buf_additional_days,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.work_buf_additional_days,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.work_buf_additional_days = 1.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasMaxNCpusPct_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.max_ncpus_pct,
                                             null, null);
        globalPreferencesParser.characters("0.75".toCharArray(), 0, 4);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.max_ncpus_pct,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.max_ncpus_pct = 0.75;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasCpuSchedulingPeriodMinutes0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.cpu_scheduling_period_minutes,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.cpu_scheduling_period_minutes,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.cpu_scheduling_period_minutes = 60.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasCpuSchedulingPeriodMinutes1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.cpu_scheduling_period_minutes,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.cpu_scheduling_period_minutes,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.cpu_scheduling_period_minutes = 1.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDiskInterval_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.disk_interval,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.disk_interval,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.disk_interval = 1.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDiskMaxUsedGb_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.disk_max_used_gb,
                                             null, null);
        globalPreferencesParser.characters("10".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.disk_max_used_gb,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.disk_max_used_gb = 10.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDiskMaxUsedPct_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.disk_max_used_pct,
                                             null, null);
        globalPreferencesParser.characters("10".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.disk_max_used_pct,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.disk_max_used_pct = 10.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDiskMinFreeGb_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.disk_min_free_gb,
                                             null, null);
        globalPreferencesParser.characters("5".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.disk_min_free_gb,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.disk_min_free_gb = 5.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRamMaxUsedBusyFrac_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.ram_max_used_busy_frac,
                                             null, null);
        globalPreferencesParser.characters("0.5".toCharArray(), 0, 3);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.ram_max_used_busy_frac,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.ram_max_used_busy_frac = 0.5;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasRamMaxUsedIdleFrac_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.ram_max_used_idle_frac,
                                             null, null);
        globalPreferencesParser.characters("0.5".toCharArray(), 0, 3);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.ram_max_used_idle_frac,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.ram_max_used_idle_frac = 0.5;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasMaxBytesSecUp_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.max_bytes_sec_up,
                                             null, null);
        globalPreferencesParser.characters("0.5".toCharArray(), 0, 3);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.max_bytes_sec_up,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.max_bytes_sec_up = 0.5;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasMaxBytesSecDown_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.max_bytes_sec_down,
                                             null, null);
        globalPreferencesParser.characters("0.5".toCharArray(), 0, 3);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.max_bytes_sec_down,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.max_bytes_sec_down = 0.5;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasCpuUsageLimit_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.cpu_usage_limit,
                                             null, null);
        globalPreferencesParser.characters("60".toCharArray(), 0, 2);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.cpu_usage_limit,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.cpu_usage_limit = 60.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDailyTransferLimitMb_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.daily_xfer_limit_mb,
                                             null, null);
        globalPreferencesParser.characters("250".toCharArray(), 0, 3);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.daily_xfer_limit_mb,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.daily_xfer_limit_mb = 250.0;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasDailyTransferPeriodDays_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.daily_xfer_period_days,
                                             null, null);
        globalPreferencesParser.characters("2".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.daily_xfer_period_days,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.daily_xfer_period_days = 2;

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

        expected.cpu_times.setStartHour(7.0);

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

        expected.cpu_times.setEndHour(18.0);

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

        expected.net_times.setStartHour(7.0);

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

        expected.net_times.setEndHour(18.0);

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasOverrideFilePresent0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.override_file_present,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.override_file_present,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.override_file_present = false;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasOverrideFilePresent1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.override_file_present,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.override_file_present,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.override_file_present = true;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasNetworkWiFiOnly0_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.network_wifi_only,
                                             null, null);
        globalPreferencesParser.characters("0".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.network_wifi_only,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.network_wifi_only = false;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }

    @Test
    public void testParser_whenXmlGlobalPreferencesHasNetworkWiFiOnly1_thenExpectMatchingGlobalPreferences()
            throws SAXException {
        globalPreferencesParser.startElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                             null, null);
        globalPreferencesParser.startElement(null, GlobalPreferences.Fields.network_wifi_only,
                                             null, null);
        globalPreferencesParser.characters("1".toCharArray(), 0, 1);
        globalPreferencesParser.endElement(null, GlobalPreferences.Fields.network_wifi_only,
                                           null);
        globalPreferencesParser.endElement(null, GlobalPreferencesParser.GLOBAL_PREFERENCES_TAG,
                                           null);

        expected.network_wifi_only = true;

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
        expected.cpu_times.getWeekPrefs()[1] = expectedTimeSpan;

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
        expected.cpu_times.getWeekPrefs()[1] = expectedTimeSpan;

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
        expected.net_times.getWeekPrefs()[2] = expectedTimeSpan;

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
        expected.net_times.getWeekPrefs()[2] = expectedTimeSpan;

        assertEquals(expected, globalPreferencesParser.getGlobalPreferences());
    }
}
