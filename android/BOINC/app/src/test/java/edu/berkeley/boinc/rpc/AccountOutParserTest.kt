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
import edu.berkeley.boinc.rpc.AccountOutParser.Companion.parse
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
class AccountOutParserTest {
    private lateinit var accountOutParser: AccountOutParser
    private lateinit var expected: AccountOut
    @Before
    fun setUp() {
        accountOutParser = AccountOutParser()
        expected = AccountOut()
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    fun `When Rpc string is empty then expect uninitialized property access exception`() {
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

    @Test(expected = UninitializedPropertyAccessException::class)
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect uninitialized property access exception`() {
        accountOutParser.startElement(null, "", null, null)
        accountOutParser.accountOut
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    @Throws(SAXException::class)
    fun `When Xml Document is empty then expect uninitialized property access exception`() {
        accountOutParser.startElement(null, "<?xml", null, null)
        accountOutParser.endElement(null, "?>", null)
        accountOutParser.accountOut
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Document has invalid 'error_num' then expect default AccountOut`() {
        PowerMockito.mockStatic(Log::class.java)
        accountOutParser.startElement(null, "<?xml", null, null)
        accountOutParser.startElement(null, ERROR_NUM, null, null)
        accountOutParser.characters("One".toCharArray(), 0, 3)
        accountOutParser.endElement(null, ERROR_NUM, null)
        accountOutParser.endElement(null, "?>", null)
        Assert.assertEquals(expected, accountOutParser.accountOut)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Document has 'error_num' then expect AccountOut with 'error_num'`() {
        accountOutParser.startElement(null, "<?xml", null, null)
        accountOutParser.startElement(null, ERROR_NUM, null, null)
        accountOutParser.characters("1".toCharArray(), 0, 1)
        accountOutParser.endElement(null, ERROR_NUM, null)
        accountOutParser.endElement(null, "?>", null)
        expected.errorNum = 1
        Assert.assertEquals(expected, accountOutParser.accountOut)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Document has 'error_msg' then expect AccountOut with 'error_msg'`() {
        accountOutParser.startElement(null, "<?xml", null, null)
        accountOutParser.startElement(null, AccountOut.Fields.ERROR_MSG, null, null)
        accountOutParser.characters("Error message".toCharArray(), 0, 13)
        accountOutParser.endElement(null, AccountOut.Fields.ERROR_MSG, null)
        accountOutParser.endElement(null, "?>", null)
        expected.errorMsg = "Error message"
        Assert.assertEquals(expected, accountOutParser.accountOut)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Document has 'authenticator' then expect AccountOut with 'authenticator'`() {
        accountOutParser.startElement(null, "<?xml", null, null)
        accountOutParser.startElement(null, AccountOut.Fields.AUTHENTICATOR, null, null)
        accountOutParser.characters("Authenticator".toCharArray(), 0, 13)
        accountOutParser.endElement(null, AccountOut.Fields.AUTHENTICATOR, null)
        accountOutParser.endElement(null, "?>", null)
        expected.authenticator = "Authenticator"
        Assert.assertEquals(expected, accountOutParser.accountOut)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Document has both 'error_num' and 'error_msg' then expect AccountOut with both 'error_num' and 'error_msg'`() {
        accountOutParser.startElement(null, "<?xml", null, null)
        accountOutParser.startElement(null, ERROR_NUM, null, null)
        accountOutParser.characters("1".toCharArray(), 0, 1)
        accountOutParser.endElement(null, ERROR_NUM, null)
        accountOutParser.startElement(null, AccountOut.Fields.ERROR_MSG, null, null)
        accountOutParser.characters("Error message".toCharArray(), 0, 13)
        accountOutParser.endElement(null, AccountOut.Fields.ERROR_MSG, null)
        accountOutParser.endElement(null, "?>", null)
        expected.errorNum = 1
        expected.errorMsg = "Error message"
        Assert.assertEquals(expected, accountOutParser.accountOut)
    }

    @Test
    @Throws(SAXException::class)
    fun `When no Xml Header is present and Xml Document has 'authenticator' then expect AccountOut with 'authenticator'`() {
        accountOutParser.startElement(null, AccountOut.Fields.AUTHENTICATOR, null, null)
        accountOutParser.characters("Authenticator".toCharArray(), 0, 13)
        accountOutParser.endElement(null, AccountOut.Fields.AUTHENTICATOR, null)
        expected.authenticator = "Authenticator"
        Assert.assertEquals(expected, accountOutParser.accountOut)
    }

    @Test
    @Throws(SAXException::class)
    fun `Test with Einstein@home example Xml Document with 'error_num' and 'error_msg' filled`() {
        accountOutParser.startElement(null, "boinc_gui_rpc_reply", null, null)
        accountOutParser.startElement(null, "error", null, null)
        accountOutParser.startElement(null, "error_num", null, null)
        accountOutParser.characters("-208".toCharArray(), 0, 4)
        accountOutParser.endElement(null, "error_num", null)
        accountOutParser.startElement(null, "error_msg", null, null)
        accountOutParser.characters("Error message".toCharArray(), 0, 13)
        accountOutParser.endElement(null, "error_msg", null)
        accountOutParser.endElement(null, "error", null)
        accountOutParser.endElement(null, "boinc_gui_rpc_reply", null)
        expected.errorNum = -208
        expected.errorMsg = "Error message"
        Assert.assertEquals(expected, accountOutParser.accountOut)
    }

    @Test
    @Throws(SAXException::class)
    fun `Test with World Community Grid example Xml Document with 'error_num' and 'error_msg' filled`() {
        accountOutParser.startElement(null, "account_out", null, null)
        accountOutParser.startElement(null, "error_num", null, null)
        accountOutParser.characters("-1".toCharArray(), 0, 2)
        accountOutParser.endElement(null, "error_num", null)
        accountOutParser.startElement(null, "error_msg", null, null)
        accountOutParser.characters("Error message".toCharArray(), 0, 13)
        accountOutParser.endElement(null, "error_msg", null)
        accountOutParser.endElement(null, "account_out", null)
        expected.errorNum = -1
        expected.errorMsg = "Error message"
        Assert.assertEquals(expected, accountOutParser.accountOut)
    }
}
