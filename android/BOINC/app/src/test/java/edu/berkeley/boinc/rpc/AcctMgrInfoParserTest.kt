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
import edu.berkeley.boinc.rpc.AcctMgrInfoParser.Companion.parse
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
class AcctMgrInfoParserTest {
    private lateinit var acctMgrInfoParser: AcctMgrInfoParser
    private lateinit var expected: AcctMgrInfo
    @Before
    fun setUp() {
        acctMgrInfoParser = AcctMgrInfoParser()
        expected = AcctMgrInfo()
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    fun `When Rpc string is null then expect uninitialized property access exception`() {
        PowerMockito.mockStatic(Xml::class.java)
        parse(null)
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    fun `When Rpc string is empty then expect uninitialized property access exception`() {
        PowerMockito.mockStatic(Xml::class.java)
        parse("")
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown then expect null`() {
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
        acctMgrInfoParser.startElement(null, "", null, null)
        Assert.assertTrue(acctMgrInfoParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When both start element and end element are run then expect element not started`() {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null)
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null)
        Assert.assertFalse(acctMgrInfoParser.mElementStarted)
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect null AccountManagerInfo`() {
        acctMgrInfoParser.startElement(null, "", null, null)
        acctMgrInfoParser.accountMgrInfo
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AccountManagerInfo has no elements then expect default AccountManagerInfo`() {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null)
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null)
        Assert.assertEquals(expected, acctMgrInfoParser.accountMgrInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AccountManagerInfo has only Name then expect matching AccountManagerInfo`() {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null)
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null, null)
        acctMgrInfoParser.characters(ACCT_MGR_NAME.toCharArray(), 0, ACCT_MGR_NAME.length)
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null)
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null)
        expected.acctMgrName = ACCT_MGR_NAME
        Assert.assertEquals(expected, acctMgrInfoParser.accountMgrInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AccountManagerInfo has only Name and Url then expect matching AccountManagerInfo`() {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null)
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null, null)
        acctMgrInfoParser.characters(ACCT_MGR_NAME.toCharArray(), 0, ACCT_MGR_NAME.length)
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null)
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null, null)
        acctMgrInfoParser.characters(ACCT_MGR_URL.toCharArray(), 0, ACCT_MGR_URL.length)
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null)
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null)
        expected.acctMgrName = ACCT_MGR_NAME
        expected.acctMgrUrl = ACCT_MGR_URL
        Assert.assertEquals(expected, acctMgrInfoParser.accountMgrInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AccountManagerInfo has only Name, Url and CookieFailureUrl then expect matching AccountManagerInfo`() {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null)
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null, null)
        acctMgrInfoParser.characters(ACCT_MGR_NAME.toCharArray(), 0, ACCT_MGR_NAME.length)
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null)
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null, null)
        acctMgrInfoParser.characters(ACCT_MGR_URL.toCharArray(), 0, ACCT_MGR_URL.length)
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null)
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null)
        expected.acctMgrName = ACCT_MGR_NAME
        expected.acctMgrUrl = ACCT_MGR_URL
        Assert.assertEquals(expected, acctMgrInfoParser.accountMgrInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AccountManagerInfo has all attributes then expect matching AccountManagerInfo`() {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null)
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null, null)
        acctMgrInfoParser.characters(ACCT_MGR_NAME.toCharArray(), 0, ACCT_MGR_NAME.length)
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null)
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null, null)
        acctMgrInfoParser.characters(ACCT_MGR_URL.toCharArray(), 0, ACCT_MGR_URL.length)
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null)
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.HAVING_CREDENTIALS, null, null)
        acctMgrInfoParser.characters("true".toCharArray(), 0, 4)
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.HAVING_CREDENTIALS, null)
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null)
        expected = AcctMgrInfo(ACCT_MGR_NAME, ACCT_MGR_URL, true)
        Assert.assertEquals(expected, acctMgrInfoParser.accountMgrInfo)
    }

    companion object {
        private const val ACCT_MGR_NAME = "Account Manager Name"
        private const val ACCT_MGR_URL = "Account Manager URL"
    }
}
