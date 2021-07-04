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
import edu.berkeley.boinc.rpc.AppsParser.Companion.parse
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
class AppsParserTest {
    private lateinit var appsParser: AppsParser
    private lateinit var expected: App
    @Before
    fun setUp() {
        appsParser = AppsParser()
        expected = App()
    }

    @Test
    fun `When Rpc string is null then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        Assert.assertTrue(parse(null).isEmpty())
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertTrue(parse("").isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When only start element is run then expect element started`() {
        appsParser.startElement(null, "", null, null)
        Assert.assertTrue(appsParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When both start element and end element are run then expect element not started`() {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null)
        appsParser.endElement(null, AppsParser.APP_TAG, null)
        Assert.assertFalse(appsParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect empty list`() {
        appsParser.startElement(null, "", null, null)
        Assert.assertTrue(appsParser.apps.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml App has no elements then expect empty list`() {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null)
        appsParser.endElement(null, AppsParser.APP_TAG, null)
        Assert.assertTrue(appsParser.apps.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml App has only name then expect matching App`() {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null)
        appsParser.startElement(null, NAME, null, null)
        appsParser.characters("Name".toCharArray(), 0, 4)
        appsParser.endElement(null, NAME, null)
        appsParser.endElement(null, AppsParser.APP_TAG, null)
        expected.name = "Name"
        Assert.assertEquals(expected, appsParser.apps[0])
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml App has only name and user_friendly_name then expect matching App`() {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null)
        appsParser.startElement(null, NAME, null, null)
        appsParser.characters("Name".toCharArray(), 0, 4)
        appsParser.endElement(null, NAME, null)
        appsParser.startElement(null, edu.berkeley.boinc.rpc.USER_FRIENDLY_NAME, null, null)
        appsParser.characters(USER_FRIENDLY_NAME.toCharArray(), 0, USER_FRIENDLY_NAME.length)
        appsParser.endElement(null, edu.berkeley.boinc.rpc.USER_FRIENDLY_NAME, null)
        appsParser.endElement(null, AppsParser.APP_TAG, null)
        expected.name = "Name"
        expected.userFriendlyName = USER_FRIENDLY_NAME
        Assert.assertEquals(listOf(expected), appsParser.apps)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml App has name, user_friendly_name and invalid non_cpu_intensive then expect App without non_cpu_intensive`() {
        PowerMockito.mockStatic(Log::class.java)
        appsParser.startElement(null, AppsParser.APP_TAG, null, null)
        appsParser.startElement(null, NAME, null, null)
        appsParser.characters("Name".toCharArray(), 0, 4)
        appsParser.endElement(null, NAME, null)
        appsParser.startElement(null, edu.berkeley.boinc.rpc.USER_FRIENDLY_NAME, null, null)
        appsParser.characters(USER_FRIENDLY_NAME.toCharArray(), 0, USER_FRIENDLY_NAME.length)
        appsParser.endElement(null, edu.berkeley.boinc.rpc.USER_FRIENDLY_NAME, null)
        appsParser.startElement(null, NON_CPU_INTENSIVE, null, null)
        appsParser.characters("One".toCharArray(), 0, 3)
        appsParser.endElement(null, NON_CPU_INTENSIVE, null)
        appsParser.endElement(null, AppsParser.APP_TAG, null)
        expected.name = "Name"
        expected.userFriendlyName = USER_FRIENDLY_NAME
        Assert.assertEquals(listOf(expected), appsParser.apps)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml App has name, user_friendly_name and non_cpu_intensive then expect matching App`() {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null)
        appsParser.startElement(null, NAME, null, null)
        appsParser.characters("Name".toCharArray(), 0, 4)
        appsParser.endElement(null, NAME, null)
        appsParser.startElement(null, edu.berkeley.boinc.rpc.USER_FRIENDLY_NAME, null, null)
        appsParser.characters(USER_FRIENDLY_NAME.toCharArray(), 0, USER_FRIENDLY_NAME.length)
        appsParser.endElement(null, edu.berkeley.boinc.rpc.USER_FRIENDLY_NAME, null)
        appsParser.startElement(null, NON_CPU_INTENSIVE, null, null)
        appsParser.characters("1".toCharArray(), 0, 1)
        appsParser.endElement(null, NON_CPU_INTENSIVE, null)
        appsParser.endElement(null, AppsParser.APP_TAG, null)
        expected.name = "Name"
        expected.userFriendlyName = USER_FRIENDLY_NAME
        expected.nonCpuIntensive = 1
        Assert.assertEquals(listOf(expected), appsParser.apps)
    }

    @Test
    @Throws(SAXException::class)
    fun `When two Xml Apps have name, user_friendly_name and non_cpu_intensive then expect two matching Apps`() {
        appsParser.startElement(null, AppsParser.APP_TAG, null, null)
        appsParser.startElement(null, NAME, null, null)
        appsParser.characters("Name 1".toCharArray(), 0, 6)
        appsParser.endElement(null, NAME, null)
        appsParser.startElement(null, edu.berkeley.boinc.rpc.USER_FRIENDLY_NAME, null, null)
        appsParser.characters(
            ("$USER_FRIENDLY_NAME 1").toCharArray(), 0, (USER_FRIENDLY_NAME +
                    " 1").length
        )
        appsParser.endElement(null, edu.berkeley.boinc.rpc.USER_FRIENDLY_NAME, null)
        appsParser.startElement(null, NON_CPU_INTENSIVE, null, null)
        appsParser.characters("1".toCharArray(), 0, 1)
        appsParser.endElement(null, NON_CPU_INTENSIVE, null)
        appsParser.endElement(null, AppsParser.APP_TAG, null)
        appsParser.startElement(null, AppsParser.APP_TAG, null, null)
        appsParser.startElement(null, NAME, null, null)
        appsParser.characters("Name 2".toCharArray(), 0, 6)
        appsParser.endElement(null, NAME, null)
        appsParser.startElement(null, edu.berkeley.boinc.rpc.USER_FRIENDLY_NAME, null, null)
        appsParser.characters(
            ("$USER_FRIENDLY_NAME 2").toCharArray(), 0, (USER_FRIENDLY_NAME +
                    " 2").length
        )
        appsParser.endElement(null, edu.berkeley.boinc.rpc.USER_FRIENDLY_NAME, null)
        appsParser.startElement(null, NON_CPU_INTENSIVE, null, null)
        appsParser.characters("1".toCharArray(), 0, 1)
        appsParser.endElement(null, NON_CPU_INTENSIVE, null)
        appsParser.endElement(null, AppsParser.APP_TAG, null)
        expected.name = "Name 1"
        expected.userFriendlyName = "$USER_FRIENDLY_NAME 1"
        expected.nonCpuIntensive = 1
        val apps: List<App?> = listOf(
            expected, App(
                "Name 2",
                "$USER_FRIENDLY_NAME 2",
                1
            )
        )
        Assert.assertEquals(apps, appsParser.apps)
    }

    companion object {
        private const val USER_FRIENDLY_NAME = "User-friendly name"
    }
}
