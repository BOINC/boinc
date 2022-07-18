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
import edu.berkeley.boinc.rpc.VersionInfoParser.Companion.parse
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
class VersionInfoParserTest {
    private lateinit var versionInfoParser: VersionInfoParser
    private lateinit var expected: VersionInfo
    @Before
    fun setUp() {
        versionInfoParser = VersionInfoParser()
        expected = VersionInfo()
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    fun `When Rpc string is null then expect UninitializedPropertyAccessException`() {
        PowerMockito.mockStatic(Xml::class.java)
        parse(null)
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

    @Test
    @Throws(SAXException::class)
    fun `When only start element is run then expect element started`() {
        versionInfoParser.startElement(null, "", null, null)
        Assert.assertTrue(versionInfoParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When both start element and end element are run then expect element not started`() {
        versionInfoParser.startElement(null, VersionInfoParser.SERVER_VERSION_TAG, null, null)
        versionInfoParser.endElement(null, VersionInfoParser.SERVER_VERSION_TAG, null)
        Assert.assertFalse(versionInfoParser.mElementStarted)
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect UninitializedPropertyAccessException`() {
        versionInfoParser.startElement(null, "", null, null)
        versionInfoParser.versionInfo
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is server_version tag then expect default VersionInfo`() {
        versionInfoParser.startElement(null, VersionInfoParser.SERVER_VERSION_TAG, null, null)
        versionInfoParser.endElement(null, VersionInfoParser.SERVER_VERSION_TAG, null)
        Assert.assertEquals(expected, versionInfoParser.versionInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml VersionInfo has invalid major_version then expect VersionInfo without major_version`() {
        PowerMockito.mockStatic(Log::class.java)
        versionInfoParser.startElement(null, VersionInfoParser.SERVER_VERSION_TAG, null, null)
        versionInfoParser.startElement(null, VersionInfo.Fields.MAJOR, null, null)
        versionInfoParser.characters("One".toCharArray(), 0, 3)
        versionInfoParser.endElement(null, VersionInfo.Fields.MAJOR, null)
        versionInfoParser.endElement(null, VersionInfoParser.SERVER_VERSION_TAG, null)
        Assert.assertEquals(expected, versionInfoParser.versionInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml VersionInfo has major_version then expect matching VersionInfo`() {
        versionInfoParser.startElement(null, VersionInfoParser.SERVER_VERSION_TAG, null, null)
        versionInfoParser.startElement(null, VersionInfo.Fields.MAJOR, null, null)
        versionInfoParser.characters("1".toCharArray(), 0, 1)
        versionInfoParser.endElement(null, VersionInfo.Fields.MAJOR, null)
        versionInfoParser.endElement(null, VersionInfoParser.SERVER_VERSION_TAG, null)
        expected.major = 1
        Assert.assertEquals(expected, versionInfoParser.versionInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml VersionInfo has major and minor versions then expect matching VersionInfo`() {
        versionInfoParser.startElement(null, VersionInfoParser.SERVER_VERSION_TAG, null, null)
        versionInfoParser.startElement(null, VersionInfo.Fields.MAJOR, null, null)
        versionInfoParser.characters("1".toCharArray(), 0, 1)
        versionInfoParser.endElement(null, VersionInfo.Fields.MAJOR, null)
        versionInfoParser.startElement(null, VersionInfo.Fields.MINOR, null, null)
        versionInfoParser.characters("1".toCharArray(), 0, 1)
        versionInfoParser.endElement(null, VersionInfo.Fields.MINOR, null)
        versionInfoParser.endElement(null, VersionInfoParser.SERVER_VERSION_TAG, null)
        expected.major = 1
        expected.minor = 1
        Assert.assertEquals(expected, versionInfoParser.versionInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml VersionInfo has major, minor and release versions then expect matching VersionInfo`() {
        versionInfoParser.startElement(null, VersionInfoParser.SERVER_VERSION_TAG, null, null)
        versionInfoParser.startElement(null, VersionInfo.Fields.MAJOR, null, null)
        versionInfoParser.characters("1".toCharArray(), 0, 1)
        versionInfoParser.endElement(null, VersionInfo.Fields.MAJOR, null)
        versionInfoParser.startElement(null, VersionInfo.Fields.MINOR, null, null)
        versionInfoParser.characters("1".toCharArray(), 0, 1)
        versionInfoParser.endElement(null, VersionInfo.Fields.MINOR, null)
        versionInfoParser.startElement(null, VersionInfo.Fields.RELEASE, null, null)
        versionInfoParser.characters("1".toCharArray(), 0, 1)
        versionInfoParser.endElement(null, VersionInfo.Fields.RELEASE, null)
        versionInfoParser.endElement(null, VersionInfoParser.SERVER_VERSION_TAG, null)
        expected.major = 1
        expected.minor = 1
        expected.release = 1
        Assert.assertEquals(expected, versionInfoParser.versionInfo)
    }
}
