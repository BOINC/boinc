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

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class AppsParserTest {
    private static final String USER_FRIENDLY_NAME = "User-friendly name";

    private AppsParser appsParser;

    @Before
    public void setUp() {
        appsParser = new AppsParser();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectEmptyList() {
        mockStatic(Xml.class);

        assertEquals(Collections.emptyList(), AppsParser.parse(null));
    }

    @Test
    public void testParser_whenOnlyStartElementIsRun_thenExpectElementStarted() throws SAXException {
        appsParser.startElement(null, "", null, null);

        assertTrue(appsParser.mElementStarted);
    }

    @Test
    public void testParser_whenBothStartElementAndEndElementAreRun_thenExpectElementNotStarted() throws SAXException {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        assertFalse(appsParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectEmptyList() throws SAXException {
        appsParser.startElement(null, "", null, null);

        assertTrue(appsParser.getApps().isEmpty());
    }

    @Test
    public void testParser_whenXmlAppHasNoElements_thenExpectEmptyList()
            throws SAXException {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        assertTrue(appsParser.getApps().isEmpty());
    }

    @Test
    public void testParser_whenXmlAppHasOnlyName_thenExpectMatchingApp()
            throws SAXException {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null);
        appsParser.startElement(null, App.Fields.name, null, null);
        appsParser.characters("Name".toCharArray(), 0, 4);
        appsParser.endElement(null, App.Fields.name, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        assertEquals(Collections.singletonList(new App("Name")),  appsParser.getApps());
    }

    @Test
    public void testParser_whenXmlAppHasOnlyNameAndUserFriendlyName_thenExpectMatchingApp()
            throws SAXException {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null);
        appsParser.startElement(null, App.Fields.name, null, null);
        appsParser.characters("Name".toCharArray(), 0, 4);
        appsParser.endElement(null, App.Fields.name, null);
        appsParser.startElement(null, App.Fields.user_friendly_name, null, null);
        appsParser.characters(USER_FRIENDLY_NAME.toCharArray(), 0, USER_FRIENDLY_NAME.length());
        appsParser.endElement(null, App.Fields.user_friendly_name, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        assertEquals(Collections.singletonList(new App("Name", USER_FRIENDLY_NAME)),
                     appsParser.getApps());
    }

    @Test
    public void testParser_whenXmlAppHasNameUserFriendlyNameAndNonCpuIntensive_thenExpectMatchingApp()
            throws SAXException {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null);
        appsParser.startElement(null, App.Fields.name, null, null);
        appsParser.characters("Name".toCharArray(), 0, 4);
        appsParser.endElement(null, App.Fields.name, null);
        appsParser.startElement(null, App.Fields.user_friendly_name, null, null);
        appsParser.characters(USER_FRIENDLY_NAME.toCharArray(), 0, USER_FRIENDLY_NAME.length());
        appsParser.endElement(null, App.Fields.user_friendly_name, null);
        appsParser.startElement(null, App.Fields.non_cpu_intensive, null, null);
        appsParser.characters("1".toCharArray(), 0, 1);
        appsParser.endElement(null, App.Fields.non_cpu_intensive, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        assertEquals(Collections.singletonList(new App("Name", USER_FRIENDLY_NAME, 1)),
                     appsParser.getApps());
    }

    @Test
    public void testParser_whenTwoXmlAppsHaveNameUserFriendlyNameAndNonCpuIntensive_thenExpectTwoMatchingApps()
            throws SAXException {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null);
        appsParser.startElement(null, App.Fields.name, null, null);
        appsParser.characters("Name 1".toCharArray(), 0, 6);
        appsParser.endElement(null, App.Fields.name, null);
        appsParser.startElement(null, App.Fields.user_friendly_name, null, null);
        appsParser.characters((USER_FRIENDLY_NAME + " 1").toCharArray(), 0, (USER_FRIENDLY_NAME + " 1").length());
        appsParser.endElement(null, App.Fields.user_friendly_name, null);
        appsParser.startElement(null, App.Fields.non_cpu_intensive, null, null);
        appsParser.characters("1".toCharArray(), 0, 1);
        appsParser.endElement(null, App.Fields.non_cpu_intensive, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        appsParser.startElement(null, AppsParser.APP_TAG, null, null);
        appsParser.startElement(null, App.Fields.name, null, null);
        appsParser.characters("Name 2".toCharArray(), 0, 6);
        appsParser.endElement(null, App.Fields.name, null);
        appsParser.startElement(null, App.Fields.user_friendly_name, null, null);
        appsParser.characters((USER_FRIENDLY_NAME + " 2").toCharArray(), 0, (USER_FRIENDLY_NAME + " 2").length());
        appsParser.endElement(null, App.Fields.user_friendly_name, null);
        appsParser.startElement(null, App.Fields.non_cpu_intensive, null, null);
        appsParser.characters("1".toCharArray(), 0, 1);
        appsParser.endElement(null, App.Fields.non_cpu_intensive, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        List<App> apps = Arrays.asList(new App("Name 1", USER_FRIENDLY_NAME + " 1",
                                               1),
                                       new App("Name 2", USER_FRIENDLY_NAME + " 2",
                                               1));

        assertEquals(apps, appsParser.getApps());
    }
}