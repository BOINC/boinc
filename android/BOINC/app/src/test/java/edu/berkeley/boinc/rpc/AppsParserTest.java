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
package edu.berkeley.boinc.rpc;

import android.util.Log;
import android.util.Xml;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.xml.sax.ContentHandler;
import org.xml.sax.SAXException;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.doThrow;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest({Log.class, Xml.class})
public class AppsParserTest {
    private static final String USER_FRIENDLY_NAME = "User-friendly name";

    private AppsParser appsParser;
    private App expected;

    @Before
    public void setUp() {
        appsParser = new AppsParser();
        expected = new App();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectEmptyList() {
        mockStatic(Xml.class);

        assertTrue(AppsParser.parse(null).isEmpty());
    }

    @Test
    public void testParse_whenSAXExceptionIsThrown_thenExpectEmptyList() throws Exception {
        mockStatic(Xml.class);
        mockStatic(Log.class);

        doThrow(new SAXException()).when(Xml.class, "parse", anyString(), any(ContentHandler.class));

        assertTrue(AppsParser.parse("").isEmpty());
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
        appsParser.startElement(null, RPCCommonTags.NAME, null, null);
        appsParser.characters("Name".toCharArray(), 0, 4);
        appsParser.endElement(null, RPCCommonTags.NAME, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        expected.setName("Name");

        assertEquals(expected, appsParser.getApps().get(0));
    }

    @Test
    public void testParser_whenXmlAppHasOnlyNameAndUserFriendlyName_thenExpectMatchingApp()
            throws SAXException {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null);
        appsParser.startElement(null, RPCCommonTags.NAME, null, null);
        appsParser.characters("Name".toCharArray(), 0, 4);
        appsParser.endElement(null, RPCCommonTags.NAME, null);
        appsParser.startElement(null, RPCCommonTags.USER_FRIENDLY_NAME, null, null);
        appsParser.characters(USER_FRIENDLY_NAME.toCharArray(), 0, USER_FRIENDLY_NAME.length());
        appsParser.endElement(null, RPCCommonTags.USER_FRIENDLY_NAME, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        expected.setName("Name");
        expected.setUserFriendlyName(USER_FRIENDLY_NAME);

        assertEquals(Collections.singletonList(expected), appsParser.getApps());
    }

    @Test
    public void testParser_whenXmlAppHasNameUserFriendlyNameAndInvalidNonCpuIntensive_thenExpectAppWithoutNonCpuIntensive()
            throws SAXException {
        mockStatic(Log.class);

        appsParser.startElement(null, AppsParser.APP_TAG, null, null);
        appsParser.startElement(null, RPCCommonTags.NAME, null, null);
        appsParser.characters("Name".toCharArray(), 0, 4);
        appsParser.endElement(null, RPCCommonTags.NAME, null);
        appsParser.startElement(null, RPCCommonTags.USER_FRIENDLY_NAME, null, null);
        appsParser.characters(USER_FRIENDLY_NAME.toCharArray(), 0, USER_FRIENDLY_NAME.length());
        appsParser.endElement(null, RPCCommonTags.USER_FRIENDLY_NAME, null);
        appsParser.startElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null, null);
        appsParser.characters("One".toCharArray(), 0, 3);
        appsParser.endElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        expected.setName("Name");
        expected.setUserFriendlyName(USER_FRIENDLY_NAME);

        assertEquals(Collections.singletonList(expected), appsParser.getApps());
    }

    @Test
    public void testParser_whenXmlAppHasNameUserFriendlyNameAndNonCpuIntensive_thenExpectMatchingApp()
            throws SAXException {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null);
        appsParser.startElement(null, RPCCommonTags.NAME, null, null);
        appsParser.characters("Name".toCharArray(), 0, 4);
        appsParser.endElement(null, RPCCommonTags.NAME, null);
        appsParser.startElement(null, RPCCommonTags.USER_FRIENDLY_NAME, null, null);
        appsParser.characters(USER_FRIENDLY_NAME.toCharArray(), 0, USER_FRIENDLY_NAME.length());
        appsParser.endElement(null, RPCCommonTags.USER_FRIENDLY_NAME, null);
        appsParser.startElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null, null);
        appsParser.characters("1".toCharArray(), 0, 1);
        appsParser.endElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        expected.setName("Name");
        expected.setUserFriendlyName(USER_FRIENDLY_NAME);
        expected.setNonCpuIntensive(1);

        assertEquals(Collections.singletonList(expected), appsParser.getApps());
    }

    @Test
    public void testParser_whenTwoXmlAppsHaveNameUserFriendlyNameAndNonCpuIntensive_thenExpectTwoMatchingApps()
            throws SAXException {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null);
        appsParser.startElement(null, RPCCommonTags.NAME, null, null);
        appsParser.characters("Name 1".toCharArray(), 0, 6);
        appsParser.endElement(null, RPCCommonTags.NAME, null);
        appsParser.startElement(null, RPCCommonTags.USER_FRIENDLY_NAME, null, null);
        appsParser.characters((USER_FRIENDLY_NAME + " 1").toCharArray(), 0, (USER_FRIENDLY_NAME +
                                                                             " 1").length());
        appsParser.endElement(null, RPCCommonTags.USER_FRIENDLY_NAME, null);
        appsParser.startElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null, null);
        appsParser.characters("1".toCharArray(), 0, 1);
        appsParser.endElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        appsParser.startElement(null, AppsParser.APP_TAG, null, null);
        appsParser.startElement(null, RPCCommonTags.NAME, null, null);
        appsParser.characters("Name 2".toCharArray(), 0, 6);
        appsParser.endElement(null, RPCCommonTags.NAME, null);
        appsParser.startElement(null, RPCCommonTags.USER_FRIENDLY_NAME, null, null);
        appsParser.characters((USER_FRIENDLY_NAME + " 2").toCharArray(), 0, (USER_FRIENDLY_NAME +
                                                                             " 2").length());
        appsParser.endElement(null, RPCCommonTags.USER_FRIENDLY_NAME, null);
        appsParser.startElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null, null);
        appsParser.characters("1".toCharArray(), 0, 1);
        appsParser.endElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null);
        appsParser.endElement(null, AppsParser.APP_TAG, null);

        expected.setName("Name 1");
        expected.setUserFriendlyName(USER_FRIENDLY_NAME + " 1");
        expected.setNonCpuIntensive(1);

        List<App> apps = Arrays.asList(expected, new App("Name 2",
                                                         USER_FRIENDLY_NAME + " 2",
                                                         1));

        assertEquals(apps, appsParser.getApps());
    }
}
