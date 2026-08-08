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
import edu.berkeley.boinc.rpc.AppVersionsParser.Companion.parse
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
class AppVersionsParserTest {
    private lateinit var appVersionsParser: AppVersionsParser
    private lateinit var expected: AppVersion
    @Before
    fun setUp() {
        appVersionsParser = AppVersionsParser()
        expected = AppVersion()
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
    fun `When 'localName' is empty then expect empty list`() {
        appVersionsParser.startElement(null, "", null, null)
        Assert.assertTrue(appVersionsParser.appVersions.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AppVersion has no elements then expect empty list`() {
        appVersionsParser.startElement(null, AppVersionsParser.APP_VERSION_TAG, null, null)
        appVersionsParser.endElement(null, AppVersionsParser.APP_VERSION_TAG, null)
        Assert.assertTrue(appVersionsParser.appVersions.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AppVersion has only app_name then expect matching AppVersion`() {
        appVersionsParser.startElement(null, AppVersionsParser.APP_VERSION_TAG, null, null)
        appVersionsParser.startElement(null, AppVersion.Fields.APP_NAME, null, null)
        appVersionsParser.characters(APP_NAME.toCharArray(), 0, APP_NAME.length)
        appVersionsParser.endElement(null, AppVersion.Fields.APP_NAME, null)
        appVersionsParser.endElement(null, AppVersionsParser.APP_VERSION_TAG, null)
        expected.appName = APP_NAME
        Assert.assertEquals(listOf(expected), appVersionsParser.appVersions)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AppVersion has app_name and invalid version_num then expect AppVersion without version_num`() {
        PowerMockito.mockStatic(Log::class.java)
        appVersionsParser.startElement(null, AppVersionsParser.APP_VERSION_TAG, null, null)
        appVersionsParser.startElement(null, AppVersion.Fields.APP_NAME, null, null)
        appVersionsParser.characters(APP_NAME.toCharArray(), 0, APP_NAME.length)
        appVersionsParser.endElement(null, AppVersion.Fields.APP_NAME, null)
        appVersionsParser.startElement(null, AppVersion.Fields.VERSION_NUM, null, null)
        appVersionsParser.characters("One".toCharArray(), 0, 3)
        appVersionsParser.endElement(null, AppVersion.Fields.VERSION_NUM, null)
        appVersionsParser.endElement(null, AppVersionsParser.APP_VERSION_TAG, null)
        expected.appName = APP_NAME
        Assert.assertEquals(listOf(expected), appVersionsParser.appVersions)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml AppVersion has app_name and version_num then expect matching AppVersion`() {
        appVersionsParser.startElement(null, AppVersionsParser.APP_VERSION_TAG, null, null)
        appVersionsParser.startElement(null, AppVersion.Fields.APP_NAME, null, null)
        appVersionsParser.characters(APP_NAME.toCharArray(), 0, APP_NAME.length)
        appVersionsParser.endElement(null, AppVersion.Fields.APP_NAME, null)
        appVersionsParser.startElement(null, AppVersion.Fields.VERSION_NUM, null, null)
        appVersionsParser.characters("1".toCharArray(), 0, 1)
        appVersionsParser.endElement(null, AppVersion.Fields.VERSION_NUM, null)
        appVersionsParser.endElement(null, AppVersionsParser.APP_VERSION_TAG, null)
        expected.appName = APP_NAME
        expected.versionNum = 1
        Assert.assertEquals(listOf(expected), appVersionsParser.appVersions)
    }

    @Test
    @Throws(SAXException::class)
    fun `When two Xml AppVersions have app_name and version_num then expect two matching AppVersions`() {
        appVersionsParser.startElement(null, AppVersionsParser.APP_VERSION_TAG, null, null)
        appVersionsParser.startElement(null, AppVersion.Fields.APP_NAME, null, null)
        appVersionsParser.characters(("$APP_NAME 1").toCharArray(), 0, ("$APP_NAME 1").length)
        appVersionsParser.endElement(null, AppVersion.Fields.APP_NAME, null)
        appVersionsParser.startElement(null, AppVersion.Fields.VERSION_NUM, null, null)
        appVersionsParser.characters("1".toCharArray(), 0, 1)
        appVersionsParser.endElement(null, AppVersion.Fields.VERSION_NUM, null)
        appVersionsParser.endElement(null, AppVersionsParser.APP_VERSION_TAG, null)
        appVersionsParser.startElement(null, AppVersionsParser.APP_VERSION_TAG, null, null)
        appVersionsParser.startElement(null, AppVersion.Fields.APP_NAME, null, null)
        appVersionsParser.characters(("$APP_NAME 2").toCharArray(), 0, ("$APP_NAME 2").length)
        appVersionsParser.endElement(null, AppVersion.Fields.APP_NAME, null)
        appVersionsParser.startElement(null, AppVersion.Fields.VERSION_NUM, null, null)
        appVersionsParser.characters("1".toCharArray(), 0, 1)
        appVersionsParser.endElement(null, AppVersion.Fields.VERSION_NUM, null)
        appVersionsParser.endElement(null, AppVersionsParser.APP_VERSION_TAG, null)
        expected.appName = "$APP_NAME 1"
        expected.versionNum = 1
        val appVersions: List<AppVersion?> = listOf(expected, AppVersion("$APP_NAME 2", 1))
        Assert.assertEquals(appVersions, appVersionsParser.appVersions)
    }

    companion object {
        private const val APP_NAME = "App Name"
    }
}
