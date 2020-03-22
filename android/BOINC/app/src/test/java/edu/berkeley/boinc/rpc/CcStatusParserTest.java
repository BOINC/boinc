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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class CcStatusParserTest {
    private CcStatusParser ccStatusParser;

    @Before
    public void setUp() {
        ccStatusParser = new CcStatusParser();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectNull() {
        mockStatic(Xml.class);

        assertNull(CcStatusParser.parse(null));
    }

    @Test
    public void testParse_whenRpcStringIsEmpty_thenExpectNull() {
        mockStatic(Xml.class);

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

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectNull() throws SAXException {
        ccStatusParser.startElement(null, "", null, null);

        assertNull(ccStatusParser.getCcStatus());
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
        ccStatusParser.startElement(null, CcStatus.Fields.task_mode, null, null);
        ccStatusParser.characters("One".toCharArray(), 0, 3);
        ccStatusParser.endElement(null, CcStatus.Fields.task_mode, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        assertEquals(new CcStatus(), ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasTaskMode_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.task_mode, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.task_mode, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.task_mode = 1;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasTaskModePerm_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.task_mode_perm, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.task_mode_perm, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.task_mode_perm = 1;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasTaskModeDelay_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.task_mode_delay, null, null);
        ccStatusParser.characters("1.5".toCharArray(), 0, 3);
        ccStatusParser.endElement(null, CcStatus.Fields.task_mode_delay, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.task_mode_delay = 1.5;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasTaskSuspendReason_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.task_suspend_reason, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.task_suspend_reason, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.task_suspend_reason = 1;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasNetworkMode_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.network_mode, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.network_mode, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.network_mode = 1;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasNetworkModePerm_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.network_mode_perm, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.network_mode_perm, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.network_mode_perm = 1;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasNetworkModeDelay_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.network_mode_delay, null, null);
        ccStatusParser.characters("1.5".toCharArray(), 0, 3);
        ccStatusParser.endElement(null, CcStatus.Fields.network_mode_delay, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.network_mode_delay = 1.5;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasNetworkSuspendReason_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.network_suspend_reason, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.network_suspend_reason, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.network_suspend_reason = 1;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasNetworkStatus_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.network_status, null, null);
        ccStatusParser.characters("1".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.network_status, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.network_status = 1;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasAmsPasswordError0_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.ams_password_error, null, null);
        ccStatusParser.characters("0".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.ams_password_error, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.ams_password_error = true;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasAmsPasswordError00_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.ams_password_error, null, null);
        ccStatusParser.characters("00".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.ams_password_error, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.ams_password_error = false;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasAmsPasswordError11_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.ams_password_error, null, null);
        ccStatusParser.characters("11".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.ams_password_error, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.ams_password_error = true;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasManagerMustQuit0_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.manager_must_quit, null, null);
        ccStatusParser.characters("0".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.manager_must_quit, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.manager_must_quit = true;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasManagerMustQuit00_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.manager_must_quit, null, null);
        ccStatusParser.characters("00".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.manager_must_quit, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.manager_must_quit = false;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasManagerMustQuit11_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.manager_must_quit, null, null);
        ccStatusParser.characters("11".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.manager_must_quit, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.manager_must_quit = true;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasDisallowAttach0_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.disallow_attach, null, null);
        ccStatusParser.characters("0".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.disallow_attach, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.disallow_attach = true;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasDisallowAttach00_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.disallow_attach, null, null);
        ccStatusParser.characters("00".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.disallow_attach, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.disallow_attach = false;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasDisallowAttach11_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.disallow_attach, null, null);
        ccStatusParser.characters("11".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.disallow_attach, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.disallow_attach = true;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasSimpleGuiOnly0_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.simple_gui_only, null, null);
        ccStatusParser.characters("0".toCharArray(), 0, 1);
        ccStatusParser.endElement(null, CcStatus.Fields.simple_gui_only, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.simple_gui_only = true;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasSimpleGuiOnly00_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.simple_gui_only, null, null);
        ccStatusParser.characters("00".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.simple_gui_only, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.simple_gui_only = false;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }

    @Test
    public void testParser_whenXmlCcStatusHasSimpleGuiOnly11_thenExpectMatchingCcStatus() throws SAXException {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null);
        ccStatusParser.startElement(null, CcStatus.Fields.simple_gui_only, null, null);
        ccStatusParser.characters("11".toCharArray(), 0, 2);
        ccStatusParser.endElement(null, CcStatus.Fields.simple_gui_only, null);
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null);

        final CcStatus expectedCcStatus = new CcStatus();
        expectedCcStatus.simple_gui_only = true;

        assertEquals(expectedCcStatus, ccStatusParser.getCcStatus());
    }
}
