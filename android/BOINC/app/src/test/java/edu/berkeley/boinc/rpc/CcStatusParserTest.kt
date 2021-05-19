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
import edu.berkeley.boinc.rpc.CcStatusParser.Companion.parse
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
class CcStatusParserTest {
    private lateinit var ccStatusParser: CcStatusParser
    private lateinit var expected: CcStatus
    @Before
    fun setUp() {
        ccStatusParser = CcStatusParser()
        expected = CcStatus()
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
    fun `When SAXException without message is thrown then expect null`() {
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertNull(parse(""))
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException with message is thrown then expect null`() {
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.doThrow(SAXException("SAX Exception")).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertNull(parse(""))
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect element started`() {
        ccStatusParser.startElement(null, "", null, null)
        Assert.assertTrue(ccStatusParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is cc_status tag then expect element not started`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        Assert.assertFalse(ccStatusParser.mElementStarted)
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect UninitializedPropertyAccessException`() {
        ccStatusParser.startElement(null, "", null, null)
        ccStatusParser.ccStatus
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is cc_status tag then expect default CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        Assert.assertEquals(CcStatus(), ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has invalid task_mode then expect default CcStatus`() {
        PowerMockito.mockStatic(Log::class.java)
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.TASK_MODE, null, null)
        ccStatusParser.characters("One".toCharArray(), 0, 3)
        ccStatusParser.endElement(null, CcStatus.Fields.TASK_MODE, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has task_mode then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.TASK_MODE, null, null)
        ccStatusParser.characters("1".toCharArray(), 0, 1)
        ccStatusParser.endElement(null, CcStatus.Fields.TASK_MODE, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.taskMode = 1
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has task_mode_perm then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.TASK_MODE_PERM, null, null)
        ccStatusParser.characters("1".toCharArray(), 0, 1)
        ccStatusParser.endElement(null, CcStatus.Fields.TASK_MODE_PERM, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.taskModePerm = 1
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has task_mode_delay then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.TASK_MODE_DELAY, null, null)
        ccStatusParser.characters("1.5".toCharArray(), 0, 3)
        ccStatusParser.endElement(null, CcStatus.Fields.TASK_MODE_DELAY, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.taskModeDelay = 1.5
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has task_suspend_reason then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.TASK_SUSPEND_REASON, null, null)
        ccStatusParser.characters("1".toCharArray(), 0, 1)
        ccStatusParser.endElement(null, CcStatus.Fields.TASK_SUSPEND_REASON, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.taskSuspendReason = 1
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has network_mode then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.NETWORK_MODE, null, null)
        ccStatusParser.characters("1".toCharArray(), 0, 1)
        ccStatusParser.endElement(null, CcStatus.Fields.NETWORK_MODE, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.networkMode = 1
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has network_mode_perm then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.NETWORK_MODE_PERM, null, null)
        ccStatusParser.characters("1".toCharArray(), 0, 1)
        ccStatusParser.endElement(null, CcStatus.Fields.NETWORK_MODE_PERM, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.networkModePerm = 1
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has network_mode_delay then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.NETWORK_MODE_DELAY, null, null)
        ccStatusParser.characters("1.5".toCharArray(), 0, 3)
        ccStatusParser.endElement(null, CcStatus.Fields.NETWORK_MODE_DELAY, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.networkModeDelay = 1.5
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has network_suspend_reason then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.NETWORK_SUSPEND_REASON, null, null)
        ccStatusParser.characters("1".toCharArray(), 0, 1)
        ccStatusParser.endElement(null, CcStatus.Fields.NETWORK_SUSPEND_REASON, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.networkSuspendReason = 1
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has network_status then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.NETWORK_STATUS, null, null)
        ccStatusParser.characters("1".toCharArray(), 0, 1)
        ccStatusParser.endElement(null, CcStatus.Fields.NETWORK_STATUS, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.networkStatus = 1
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has ams_password_error 0 then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.AMS_PASSWORD_ERROR, null, null)
        ccStatusParser.characters("0".toCharArray(), 0, 1)
        ccStatusParser.endElement(null, CcStatus.Fields.AMS_PASSWORD_ERROR, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.amsPasswordError = true
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has ams_password_error 00 then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.AMS_PASSWORD_ERROR, null, null)
        ccStatusParser.characters("00".toCharArray(), 0, 2)
        ccStatusParser.endElement(null, CcStatus.Fields.AMS_PASSWORD_ERROR, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.amsPasswordError = false
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has ams_password_error 11 then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.AMS_PASSWORD_ERROR, null, null)
        ccStatusParser.characters("11".toCharArray(), 0, 2)
        ccStatusParser.endElement(null, CcStatus.Fields.AMS_PASSWORD_ERROR, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.amsPasswordError = true
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has manager_must_quit 0 then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.MANAGER_MUST_QUIT, null, null)
        ccStatusParser.characters("0".toCharArray(), 0, 1)
        ccStatusParser.endElement(null, CcStatus.Fields.MANAGER_MUST_QUIT, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.managerMustQuit = true
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has manager_must_quit 00 then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.MANAGER_MUST_QUIT, null, null)
        ccStatusParser.characters("00".toCharArray(), 0, 2)
        ccStatusParser.endElement(null, CcStatus.Fields.MANAGER_MUST_QUIT, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.managerMustQuit = false
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has manager_must_quit 11 then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.MANAGER_MUST_QUIT, null, null)
        ccStatusParser.characters("11".toCharArray(), 0, 2)
        ccStatusParser.endElement(null, CcStatus.Fields.MANAGER_MUST_QUIT, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.managerMustQuit = true
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has disallow_attach 0 then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.DISALLOW_ATTACH, null, null)
        ccStatusParser.characters("0".toCharArray(), 0, 1)
        ccStatusParser.endElement(null, CcStatus.Fields.DISALLOW_ATTACH, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.disallowAttach = true
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has disallow_attach 00 then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.DISALLOW_ATTACH, null, null)
        ccStatusParser.characters("00".toCharArray(), 0, 2)
        ccStatusParser.endElement(null, CcStatus.Fields.DISALLOW_ATTACH, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.disallowAttach = false
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has disallow_attach 11 then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.DISALLOW_ATTACH, null, null)
        ccStatusParser.characters("11".toCharArray(), 0, 2)
        ccStatusParser.endElement(null, CcStatus.Fields.DISALLOW_ATTACH, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.disallowAttach = true
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has simple_gui_only 0 then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.SIMPLE_GUI_ONLY, null, null)
        ccStatusParser.characters("0".toCharArray(), 0, 1)
        ccStatusParser.endElement(null, CcStatus.Fields.SIMPLE_GUI_ONLY, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.simpleGuiOnly = true
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has simple_gui_only 00 then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.SIMPLE_GUI_ONLY, null, null)
        ccStatusParser.characters("00".toCharArray(), 0, 2)
        ccStatusParser.endElement(null, CcStatus.Fields.SIMPLE_GUI_ONLY, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.simpleGuiOnly = false
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcStatus has simple_gui_only 11 then expect matching CcStatus`() {
        ccStatusParser.startElement(null, CcStatusParser.CC_STATUS_TAG, null, null)
        ccStatusParser.startElement(null, CcStatus.Fields.SIMPLE_GUI_ONLY, null, null)
        ccStatusParser.characters("11".toCharArray(), 0, 2)
        ccStatusParser.endElement(null, CcStatus.Fields.SIMPLE_GUI_ONLY, null)
        ccStatusParser.endElement(null, CcStatusParser.CC_STATUS_TAG, null)
        expected.simpleGuiOnly = true
        Assert.assertEquals(expected, ccStatusParser.ccStatus)
    }
}
