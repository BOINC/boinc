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

import android.util.Xml
import edu.berkeley.boinc.rpc.AccountManagerParser.Companion.parse
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
@PrepareForTest(Xml::class)
class AccountManagerParserTest {
    private lateinit var accountManagerParser: AccountManagerParser
    private lateinit var accountManagers: List<AccountManager>
    private lateinit var expected: AccountManager
    @Before
    fun setUp() {
        accountManagerParser = AccountManagerParser()
        accountManagers = accountManagerParser.accountManagerInfos
        expected = AccountManager()
    }

    @Test
    fun `When Rpc string is null then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        val accountManagers = parse(null)
        Assert.assertNotNull(accountManagers)
        Assert.assertTrue(accountManagers.isEmpty())
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        val accountManagers = parse(null)
        Assert.assertNotNull(accountManagers)
        Assert.assertTrue(accountManagers.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect ElementStarted`() {
        accountManagerParser.startElement(null, "", null, null)
        Assert.assertTrue(accountManagerParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect empty list`() {
        accountManagerParser.startElement(null, "", null, null)
        Assert.assertTrue(accountManagers.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When one Account Manager with no elements then expect empty list`() {
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null, null)
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null)
        Assert.assertTrue(accountManagers.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Account Manager has only name without closing tag then expect ElementNotStarted`() {
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null, null)
        accountManagerParser.startElement(null, NAME, null, null)
        accountManagerParser.characters(ACCOUNT_MANAGER.toCharArray(), 0, ACCOUNT_MANAGER.length)
        accountManagerParser.endElement(null, "", null)
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null)
        Assert.assertFalse(accountManagerParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Account Manager has only name without closing tag then expect empty list`() {
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null, null)
        accountManagerParser.startElement(null, NAME, null, null)
        accountManagerParser.characters(ACCOUNT_MANAGER.toCharArray(), 0, ACCOUNT_MANAGER.length)
        accountManagerParser.endElement(null, "", null)
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null)
        Assert.assertTrue(accountManagers.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When one Account Manager with only name then expect element with only name`() {
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null, null)
        accountManagerParser.startElement(null, NAME, null, null)
        accountManagerParser.characters(ACCOUNT_MANAGER.toCharArray(), 0, ACCOUNT_MANAGER.length)
        accountManagerParser.endElement(null, NAME, null)
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null)
        expected.name = ACCOUNT_MANAGER
        Assert.assertEquals(1, accountManagers.size.toLong())
        Assert.assertEquals(expected, accountManagers[0])
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Account Manager has only name and url then expect matching Account Manager`() {
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null, null)
        accountManagerParser.startElement(null, NAME, null, null)
        accountManagerParser.characters(ACCOUNT_MANAGER.toCharArray(), 0, ACCOUNT_MANAGER.length)
        accountManagerParser.endElement(null, NAME, null)
        accountManagerParser.startElement(null, URL, null, null)
        accountManagerParser.characters("URL".toCharArray(), 0, 3)
        accountManagerParser.endElement(null, URL, null)
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null)
        expected.name = ACCOUNT_MANAGER
        expected.url = "URL"
        Assert.assertEquals(1, accountManagers.size.toLong())
        Assert.assertEquals(expected, accountManagers[0])
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Account Manager with only name, url and description then expect matching Account Manager`() {
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null, null)
        accountManagerParser.startElement(null, NAME, null, null)
        accountManagerParser.characters(ACCOUNT_MANAGER.toCharArray(), 0, ACCOUNT_MANAGER.length)
        accountManagerParser.endElement(null, NAME, null)
        accountManagerParser.startElement(null, URL, null, null)
        accountManagerParser.characters("URL".toCharArray(), 0, 3)
        accountManagerParser.endElement(null, URL, null)
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.DESCRIPTION, null, null)
        accountManagerParser.characters(DESCRIPTION.toCharArray(), 0, DESCRIPTION.length)
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.DESCRIPTION, null)
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null)
        expected.name = ACCOUNT_MANAGER
        expected.url = "URL"
        expected.description = DESCRIPTION
        Assert.assertEquals(1, accountManagers.size.toLong())
        Assert.assertEquals(expected, accountManagers[0])
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Account Manager has all attributes then expect matching Account Manager`() {
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null, null)
        accountManagerParser.startElement(null, NAME, null, null)
        accountManagerParser.characters(ACCOUNT_MANAGER.toCharArray(), 0, ACCOUNT_MANAGER.length)
        accountManagerParser.endElement(null, NAME, null)
        accountManagerParser.startElement(null, URL, null, null)
        accountManagerParser.characters("URL".toCharArray(), 0, 3)
        accountManagerParser.endElement(null, URL, null)
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.DESCRIPTION, null, null)
        accountManagerParser.characters(DESCRIPTION.toCharArray(), 0, DESCRIPTION.length)
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.DESCRIPTION, null)
        accountManagerParser.startElement(null, AccountManagerParser.IMAGE_TAG, null, null)
        accountManagerParser.characters(IMAGE_URL.toCharArray(), 0, IMAGE_URL.length)
        accountManagerParser.endElement(null, AccountManagerParser.IMAGE_TAG, null)
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null)
        expected.name = ACCOUNT_MANAGER
        expected.url = "URL"
        expected.description = DESCRIPTION
        expected.imageUrl = IMAGE_URL
        Assert.assertEquals(1, accountManagers.size.toLong())
        Assert.assertEquals(expected, accountManagers[0])
    }

    @Test
    @Throws(SAXException::class)
    fun `When two Xml Account Managers have all attributes then expect two matching AccountManagers`() {
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null, null)
        accountManagerParser.startElement(null, NAME, null, null)
        accountManagerParser.characters(
            ("$ACCOUNT_MANAGER 1").toCharArray(), 0,
            ACCOUNT_MANAGER.length + 2
        )
        accountManagerParser.endElement(null, NAME, null)
        accountManagerParser.startElement(null, URL, null, null)
        accountManagerParser.characters("URL 1".toCharArray(), 0, 5)
        accountManagerParser.endElement(null, URL, null)
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.DESCRIPTION, null, null)
        accountManagerParser.characters(
            ("$DESCRIPTION 1").toCharArray(), 0,
            DESCRIPTION.length + 2
        )
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.DESCRIPTION, null)
        accountManagerParser.startElement(null, AccountManagerParser.IMAGE_TAG, null, null)
        accountManagerParser.characters(("$IMAGE_URL 1").toCharArray(), 0, IMAGE_URL.length + 2)
        accountManagerParser.endElement(null, AccountManagerParser.IMAGE_TAG, null)
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null)
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null, null)
        accountManagerParser.startElement(null, NAME, null, null)
        accountManagerParser.characters(("$ACCOUNT_MANAGER 2").toCharArray(), 0, 17)
        accountManagerParser.endElement(null, NAME, null)
        accountManagerParser.startElement(null, URL, null, null)
        accountManagerParser.characters("URL 2".toCharArray(), 0, 5)
        accountManagerParser.endElement(null, URL, null)
        accountManagerParser.startElement(null, edu.berkeley.boinc.rpc.DESCRIPTION, null, null)
        accountManagerParser.characters(
            ("$DESCRIPTION 2").toCharArray(), 0,
            DESCRIPTION.length + 2
        )
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.DESCRIPTION, null)
        accountManagerParser.startElement(null, AccountManagerParser.IMAGE_TAG, null, null)
        accountManagerParser.characters(("$IMAGE_URL 2").toCharArray(), 0, IMAGE_URL.length + 2)
        accountManagerParser.endElement(null, AccountManagerParser.IMAGE_TAG, null)
        accountManagerParser.endElement(null, edu.berkeley.boinc.rpc.ACCOUNT_MANAGER, null)
        expected.name = "$ACCOUNT_MANAGER 1"
        expected.url = "URL 1"
        expected.description = "$DESCRIPTION 1"
        expected.imageUrl = "$IMAGE_URL 1"
        val expected2 = AccountManager(
            "$ACCOUNT_MANAGER 2",
            "URL 2",
            "$DESCRIPTION 2",
            "$IMAGE_URL 2"
        )
        Assert.assertEquals(2, accountManagers.size.toLong())
        Assert.assertEquals(expected, accountManagers[0])
        Assert.assertEquals(expected2, accountManagers[1])
    }

    companion object {
        private const val ACCOUNT_MANAGER = "Account Manager"
        private const val DESCRIPTION = "Description"
        private const val IMAGE_URL = "Image URL"
    }
}
