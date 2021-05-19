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
import edu.berkeley.boinc.rpc.AcctMgrRPCReplyParser.Companion.parse
import edu.berkeley.boinc.utils.Logging.setLogLevel
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
class AcctMgrRPCReplyParserTest {
    private lateinit var acctMgrRPCReplyParser: AcctMgrRPCReplyParser
    private lateinit var expected: AcctMgrRPCReply
    @Before
    fun setUp() {
        acctMgrRPCReplyParser = AcctMgrRPCReplyParser()
        expected = AcctMgrRPCReply()
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    fun `When Rpc string is empty then expect uninitialized property access exception`() {
        PowerMockito.mockStatic(Xml::class.java)
        parse("")
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown and log level is 1 then expect null`() {
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.mockStatic(Xml::class.java)
        setLogLevel(1)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertNull(parse(""))
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown and log level is 2 then expect null`() {
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.mockStatic(Xml::class.java)
        setLogLevel(2)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertNull(parse(""))
    }

    @Test
    @Throws(SAXException::class)
    fun `When only start element is run then expect element started`() {
        acctMgrRPCReplyParser.startElement(null, "", null, null)
        Assert.assertTrue(acctMgrRPCReplyParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When both start element and end element are run then expect element not started`() {
        acctMgrRPCReplyParser.startElement(
            null,
            AcctMgrRPCReplyParser.ACCT_MGR_RPC_REPLY_TAG,
            null,
            null
        )
        acctMgrRPCReplyParser.endElement(null, AcctMgrRPCReplyParser.ACCT_MGR_RPC_REPLY_TAG, null)
        Assert.assertFalse(acctMgrRPCReplyParser.mElementStarted)
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect uninitialized property access exception`() {
        acctMgrRPCReplyParser.startElement(null, "", null, null)
        acctMgrRPCReplyParser.accountMgrRPCReply
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AccountManagerRPCReply with no elements then expect default entity`() {
        acctMgrRPCReplyParser.startElement(
            null,
            AcctMgrRPCReplyParser.ACCT_MGR_RPC_REPLY_TAG,
            null,
            null
        )
        acctMgrRPCReplyParser.endElement(null, AcctMgrRPCReplyParser.ACCT_MGR_RPC_REPLY_TAG, null)
        Assert.assertEquals(expected, acctMgrRPCReplyParser.accountMgrRPCReply)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AccountManagerRPCReply with invalid 'error_num' then expect default entity`() {
        PowerMockito.mockStatic(Log::class.java)
        acctMgrRPCReplyParser.startElement(
            null,
            AcctMgrRPCReplyParser.ACCT_MGR_RPC_REPLY_TAG,
            null,
            null
        )
        acctMgrRPCReplyParser.startElement(null, ERROR_NUM, null, null)
        acctMgrRPCReplyParser.characters("One".toCharArray(), 0, 3)
        acctMgrRPCReplyParser.endElement(null, ERROR_NUM, null)
        acctMgrRPCReplyParser.endElement(null, AcctMgrRPCReplyParser.ACCT_MGR_RPC_REPLY_TAG, null)
        Assert.assertEquals(expected, acctMgrRPCReplyParser.accountMgrRPCReply)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AccountManagerRPCReply with only 'error_num' then expect matching entity`() {
        acctMgrRPCReplyParser.startElement(
            null,
            AcctMgrRPCReplyParser.ACCT_MGR_RPC_REPLY_TAG,
            null,
            null
        )
        acctMgrRPCReplyParser.startElement(null, ERROR_NUM, null, null)
        acctMgrRPCReplyParser.characters("1".toCharArray(), 0, 1)
        acctMgrRPCReplyParser.endElement(null, ERROR_NUM, null)
        acctMgrRPCReplyParser.endElement(null, AcctMgrRPCReplyParser.ACCT_MGR_RPC_REPLY_TAG, null)
        expected.errorNum = 1
        Assert.assertEquals(expected, acctMgrRPCReplyParser.accountMgrRPCReply)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AccountManagerRPCReply has 'error_num' and one message then expect matching entity`() {
        acctMgrRPCReplyParser.startElement(
            null,
            AcctMgrRPCReplyParser.ACCT_MGR_RPC_REPLY_TAG,
            null,
            null
        )
        acctMgrRPCReplyParser.startElement(null, ERROR_NUM, null, null)
        acctMgrRPCReplyParser.characters("1".toCharArray(), 0, 1)
        acctMgrRPCReplyParser.endElement(null, ERROR_NUM, null)
        acctMgrRPCReplyParser.startElement(null, edu.berkeley.boinc.rpc.MESSAGE, null, null)
        acctMgrRPCReplyParser.characters(MESSAGE.toCharArray(), 0, MESSAGE.length)
        acctMgrRPCReplyParser.endElement(null, edu.berkeley.boinc.rpc.MESSAGE, null)
        acctMgrRPCReplyParser.endElement(null, AcctMgrRPCReplyParser.ACCT_MGR_RPC_REPLY_TAG, null)
        expected.errorNum = 1
        expected.messages.add(MESSAGE)
        Assert.assertEquals(expected, acctMgrRPCReplyParser.accountMgrRPCReply)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AccountManagerRPCReply has 'error_num' and two messages then expect matching entity`() {
        acctMgrRPCReplyParser.startElement(
            null,
            AcctMgrRPCReplyParser.ACCT_MGR_RPC_REPLY_TAG,
            null,
            null
        )
        acctMgrRPCReplyParser.startElement(null, ERROR_NUM, null, null)
        acctMgrRPCReplyParser.characters("1".toCharArray(), 0, 1)
        acctMgrRPCReplyParser.endElement(null, ERROR_NUM, null)
        acctMgrRPCReplyParser.startElement(null, edu.berkeley.boinc.rpc.MESSAGE, null, null)
        acctMgrRPCReplyParser.characters(("$MESSAGE 1").toCharArray(), 0, MESSAGE.length + 2)
        acctMgrRPCReplyParser.endElement(null, edu.berkeley.boinc.rpc.MESSAGE, null)
        acctMgrRPCReplyParser.startElement(null, edu.berkeley.boinc.rpc.MESSAGE, null, null)
        acctMgrRPCReplyParser.characters(("$MESSAGE 2").toCharArray(), 0, MESSAGE.length + 2)
        acctMgrRPCReplyParser.endElement(null, edu.berkeley.boinc.rpc.MESSAGE, null)
        acctMgrRPCReplyParser.endElement(null, AcctMgrRPCReplyParser.ACCT_MGR_RPC_REPLY_TAG, null)
        expected.errorNum = 1
        expected.messages.add("$MESSAGE 1")
        expected.messages.add("$MESSAGE 2")
        Assert.assertEquals(expected, acctMgrRPCReplyParser.accountMgrRPCReply)
    }

    companion object {
        private const val MESSAGE = "Message"
    }
}
