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

import android.util.Log;
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
@PrepareForTest({Log.class, Xml.class})
public class CcStatusParserTest {
    private CcStatusParser ccStatusParser;
    private CcStatus expected;

    @Before
    public void setUp() {
        ccStatusParser = new CcStatusParser();
        expected = new CcStatus();
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParse_whenRpcStringIsNull_thenExpectUninitializedPropertyAccessException() {
        mockStatic(Xml.class);

        CcStatusParser.parse(null);
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParse_whenRpcStringIsEmpty_thenExpectUninitializedPropertyAccessException() {
        mockStatic(Xml.class);

        CcStatusParser.parse("");
    }

    @Test
    public void testParse_whenSAXExceptionWithoutMessageIsThrown_thenExpectNull() throws Exception {
        mockStatic(Log.class);
        mockStatic(Xml.class);

        doThrow(new SAXException()).when(Xml.class, "parse", anyString(), any(ContentHandler.class));

        assertNull(CcStatusParser.parse(""));
    }

    @Test
    public void testParse_whenSAXExceptionWithMessageIsThrown_thenExpectNull() throws Exception {
        mockStatic(Log.class);
        mockStatic(Xml.class);

        doThrow(new SAXException("SAX Exception")).when(Xml.class, "parse", anyString(), any(ContentHandler.class));

        assertNull(CcStatusParser.parse(""));
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectElementStarted() throws SAXException {
        ccStatusParser.startElement(null, "", null, null);

        assertTrue(ccStatusParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsCcStatusTag_thenExpectElementNotStarted()
            throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        assertFalse(ccStatusParser.mElementStarted);
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParser_whenLocalNameIsEmpty_thenExpectUninitializedPropertyAccessException() throws SAXException {
        ccStatusParser.startElement(null, "", null, null);

        ccStatusParser.getCcStatus();
    }

    @Test
    public void testParser_whenLocalNameIsCcStatusTag_thenExpectDefaultCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        assertEquals(new CcStatus(), ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasInvalidTaskMode_thenExpectDefaultCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.TASK_MODE, null, null);
        ccStatusParser.characters("One".toCharArray(), 0, 3);
        ccStatusParser.endElement(null, CcStatus.Fields.TASK_MODE, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasTaskMode_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.TASK_MODE, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.TASK_MODE, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setTaskMode(1);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasTaskModePerm_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.TASK_MODE_PERM, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.TASK_MODE_PERM, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setTaskModePerm(1);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasTaskModeDelay_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.TASK_MODE_DELAY, null, null);
        ccStatusParser.characters("1.5".toCharArray(), 0, 3);
        ccStatusParser.endElement(null, CcStatus.Fields.TASK_MODE_DELAY, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setTaskModeDelay(1.5);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasTaskSuspendReason_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.TASK_SUSPEND_REASON, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.TASK_SUSPEND_REASON, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setTaskSuspendReason(1);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasNetworkMode_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.NETWORK_MODE, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.NETWORK_MODE, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setNetworkMode(1);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasNetworkModePerm_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.NETWORK_MODE_PERM, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.NETWORK_MODE_PERM, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setNetworkModePerm(1);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasNetworkModeDelay_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.NETWORK_MODE_DELAY, null, null);
        ccStatusParser.characters("1.5".toCharArray(), 0, 3);
        ccStatusParser.endElement(null, CcStatus.Fields.NETWORK_MODE_DELAY, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setNetworkModeDelay(1.5);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasNetworkSuspendReason_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.NETWORK_SUSPEND_REASON, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.NETWORK_SUSPEND_REASON, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setNetworkSuspendReason(1);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasNetworkStatus_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.NETWORK_STATUS, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.NETWORK_STATUS, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setNetworkStatus(1);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasAmsPasswordError0_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.AMS_PASSWORD_ERROR, null, null);
        ccStatusParser.characters("0".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.AMS_PASSWORD_ERROR, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setAmsPasswordError(true);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasAmsPasswordError00_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.AMS_PASSWORD_ERROR, null, null);
        ccStatusParser.characters("00".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.AMS_PASSWORD_ERROR, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setAmsPasswordError(false);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasAmsPasswordError11_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.AMS_PASSWORD_ERROR, null, null);
        ccStatusParser.characters("11".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.AMS_PASSWORD_ERROR, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setAmsPasswordError(true);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasManagerMustQuit0_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.MANAGER_MUST_QUIT, null, null);
        ccStatusParser.characters("0".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.MANAGER_MUST_QUIT, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setManagerMustQuit(true);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasManagerMustQuit00_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.MANAGER_MUST_QUIT, null, null);
        ccStatusParser.characters("00".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.MANAGER_MUST_QUIT, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setManagerMustQuit(false);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasManagerMustQuit11_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.MANAGER_MUST_QUIT, null, null);
        ccStatusParser.characters("11".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.MANAGER_MUST_QUIT, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setManagerMustQuit(true);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasDisallowAttach0_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.DISALLOW_ATTACH, null, null);
        ccStatusParser.characters("0".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.DISALLOW_ATTACH, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setDisallowAttach(true);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasDisallowAttach00_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.DISALLOW_ATTACH, null, null);
        ccStatusParser.characters("00".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.DISALLOW_ATTACH, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setDisallowAttach(false);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasDisallowAttach11_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.DISALLOW_ATTACH, null, null);
        ccStatusParser.characters("11".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.DISALLOW_ATTACH, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setDisallowAttach(true);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasSimpleGuiOnly0_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.SIMPLE_GUI_ONLY, null, null);
        ccStatusParser.characters("0".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.SIMPLE_GUI_ONLY, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setSimpleGuiOnly(true);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasSimpleGuiOnly00_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.SIMPLE_GUI_ONLY, null, null);
        ccStatusParser.characters("00".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.SIMPLE_GUI_ONLY, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setSimpleGuiOnly(false);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasSimpleGuiOnly11_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.SIMPLE_GUI_ONLY, null, null);
        ccStatusParser.characters("11".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.SIMPLE_GUI_ONLY, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        expected.setSimpleGuiOnly(true);

        assertEquals(expected, ccStatusParser.getCcStatus());
    }
}
