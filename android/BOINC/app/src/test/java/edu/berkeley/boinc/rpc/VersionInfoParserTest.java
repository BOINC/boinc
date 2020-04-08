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
import org.xml.sax.ContentHandler;
import org.xml.sax.SAXException;

import kotlin.UninitializedPropertyAccessException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.doThrow;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class VersionInfoParserTest {
    private VersionInfoParser versionInfoParser;
    private VersionInfo expected;

    @Before
    public void setUp() {
        versionInfoParser = new VersionInfoParser();
        expected = new VersionInfo();
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParse_whenRpcStringIsNull_thenExpectUninitializedPropertyAccessException() {
        mockStatic(Xml.class);

        VersionInfoParser.parse(null);
    }

    @Test
    public void testParse_whenSAXExceptionIsThrown_thenExpectNull() throws Exception {
        mockStatic(Xml.class);

        doThrow(new SAXException()).when(Xml.class, "parse", anyString(), any(ContentHandler.class));

        assertNull(VersionInfoParser.parse(""));
    }

    @Test
    public void testParser_whenOnlyStartElementIsRun_thenExpectElementStarted() throws SAXException {
        versionInfoParser.startElement(null, "", null, null);

        assertTrue(versionInfoParser.mElementStarted);
    }

    @Test
    public void testParser_whenBothStartElementAndEndElementAreRun_thenExpectElementNotStarted()
            throws SAXException {
        versionInfoParser.startElement(null, VersionInfoParser.SERVER_VERSION_TAG, null, null);
        versionInfoParser.endElement(null, VersionInfoParser.SERVER_VERSION_TAG, null);

        assertFalse(versionInfoParser.mElementStarted);
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParser_whenLocalNameIsEmpty_thenExpectUninitializedPropertyAccessException()
            throws SAXException {
        versionInfoParser.startElement(null, "", null, null);

        versionInfoParser.getVersionInfo();
    }

    @Test
    public void testParser_whenLocalNameIsServerVersionTag_thenExpectDefaultVersionInfo() throws SAXException {
        versionInfoParser.startElement(null, VersionInfoParser.SERVER_VERSION_TAG, null, null);
        versionInfoParser.endElement(null, VersionInfoParser.SERVER_VERSION_TAG, null);

        assertEquals(expected, versionInfoParser.getVersionInfo());
    }

    @Test
    public void testParser_whenXmlVersionInfoHasInvalidMajorVersion_thenExpectVersionInfoWithoutMajorVersion()
            throws SAXException {
        versionInfoParser.startElement(null, VersionInfoParser.SERVER_VERSION_TAG, null, null);
        versionInfoParser.startElement(null, VersionInfo.Fields.MAJOR, null, null);
        versionInfoParser.characters("One".toCharArray(), 0, 3);
        versionInfoParser.endElement(null, VersionInfo.Fields.MAJOR, null);
        versionInfoParser.endElement(null, VersionInfoParser.SERVER_VERSION_TAG, null);

        assertEquals(expected, versionInfoParser.getVersionInfo());
    }

    @Test
    public void testParser_whenXmlVersionInfoHasMajorVersion_thenExpectMatchingVersionInfo()
            throws SAXException {
        versionInfoParser.startElement(null, VersionInfoParser.SERVER_VERSION_TAG, null, null);
        versionInfoParser.startElement(null, VersionInfo.Fields.MAJOR, null, null);
        versionInfoParser.characters("1".toCharArray(), 0, 1);
        versionInfoParser.endElement(null, VersionInfo.Fields.MAJOR, null);
        versionInfoParser.endElement(null, VersionInfoParser.SERVER_VERSION_TAG, null);

        expected.setMajor(1);

        assertEquals(expected, versionInfoParser.getVersionInfo());
    }

    @Test
    public void testParser_whenXmlVersionInfoHasMajorAndMinorVersions_thenExpectMatchingVersionInfo()
            throws SAXException {
        versionInfoParser.startElement(null, VersionInfoParser.SERVER_VERSION_TAG, null, null);
        versionInfoParser.startElement(null, VersionInfo.Fields.MAJOR, null, null);
        versionInfoParser.characters("1".toCharArray(), 0, 1);
        versionInfoParser.endElement(null, VersionInfo.Fields.MAJOR, null);
        versionInfoParser.startElement(null, VersionInfo.Fields.MINOR, null, null);
        versionInfoParser.characters("1".toCharArray(), 0, 1);
        versionInfoParser.endElement(null, VersionInfo.Fields.MINOR, null);
        versionInfoParser.endElement(null, VersionInfoParser.SERVER_VERSION_TAG, null);

        expected.setMajor(1);
        expected.setMinor(1);

        assertEquals(expected, versionInfoParser.getVersionInfo());
    }

    @Test
    public void testParser_whenXmlVersionInfoHasMajorMinorAndReleaseVersions_thenExpectMatchingVersionInfo()
            throws SAXException {
        versionInfoParser.startElement(null, VersionInfoParser.SERVER_VERSION_TAG, null, null);
        versionInfoParser.startElement(null, VersionInfo.Fields.MAJOR, null, null);
        versionInfoParser.characters("1".toCharArray(), 0, 1);
        versionInfoParser.endElement(null, VersionInfo.Fields.MAJOR, null);
        versionInfoParser.startElement(null, VersionInfo.Fields.MINOR, null, null);
        versionInfoParser.characters("1".toCharArray(), 0, 1);
        versionInfoParser.endElement(null, VersionInfo.Fields.MINOR, null);
        versionInfoParser.startElement(null, VersionInfo.Fields.RELEASE, null, null);
        versionInfoParser.characters("1".toCharArray(), 0, 1);
        versionInfoParser.endElement(null, VersionInfo.Fields.RELEASE, null);
        versionInfoParser.endElement(null, VersionInfoParser.SERVER_VERSION_TAG, null);

        expected.setMajor(1);
        expected.setMinor(1);
        expected.setRelease(1);

        assertEquals(expected, versionInfoParser.getVersionInfo());
    }
}
