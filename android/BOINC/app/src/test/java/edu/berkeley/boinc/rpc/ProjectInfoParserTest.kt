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
import edu.berkeley.boinc.rpc.ProjectInfoParser.Companion.parse
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
class ProjectInfoParserTest {
    private lateinit var projectInfoParser: ProjectInfoParser
    private lateinit var expected: ProjectInfo
    @Before
    fun setUp() {
        projectInfoParser = ProjectInfoParser()
        expected = ProjectInfo()
    }

    @Test
    fun `When Rpc string is empty then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        Assert.assertTrue(parse("").isEmpty())
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown then expect empty list`() {
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertTrue(parse("").isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect element started`() {
        projectInfoParser.startElement(null, "", null, null)
        Assert.assertTrue(projectInfoParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is project tag then expect element not started`() {
        projectInfoParser.startElement(null, PROJECT, null, null)
        projectInfoParser.endElement(null, PROJECT, null)
        Assert.assertFalse(projectInfoParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect empty list`() {
        projectInfoParser.startElement(null, "", null, null)
        Assert.assertTrue(projectInfoParser.projectInfos.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectInfo has no elements then expect empty list`() {
        projectInfoParser.startElement(null, PROJECT, null, null)
        projectInfoParser.endElement(null, PROJECT, null)
        Assert.assertTrue(projectInfoParser.projectInfos.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectInfo has name then expect list with matching ProjectInfo`() {
        projectInfoParser.startElement(null, PROJECT, null, null)
        projectInfoParser.startElement(null, NAME, null, null)
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length)
        projectInfoParser.endElement(null, NAME, null)
        projectInfoParser.endElement(null, PROJECT, null)
        expected.name = PROJECT_INFO_NAME
        Assert.assertEquals(listOf(expected), projectInfoParser.projectInfos)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectInfo has name and url then expect list with matching ProjectInfo`() {
        projectInfoParser.startElement(null, PROJECT, null, null)
        projectInfoParser.startElement(null, NAME, null, null)
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length)
        projectInfoParser.endElement(null, NAME, null)
        projectInfoParser.startElement(null, URL, null, null)
        projectInfoParser.characters("Project URL".toCharArray(), 0, "Project URL".length)
        projectInfoParser.endElement(null, URL, null)
        projectInfoParser.endElement(null, PROJECT, null)
        expected.name = PROJECT_INFO_NAME
        expected.url = "Project URL"
        Assert.assertEquals(listOf(expected), projectInfoParser.projectInfos)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectInfo has name and general_area then expect list with matching ProjectInfo`() {
        projectInfoParser.startElement(null, PROJECT, null, null)
        projectInfoParser.startElement(null, NAME, null, null)
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length)
        projectInfoParser.endElement(null, NAME, null)
        projectInfoParser.startElement(
            null, ProjectInfo.Fields.GENERAL_AREA, null,
            null
        )
        projectInfoParser.characters(
            "Project General Area".toCharArray(), 0,
            "Project General Area".length
        )
        projectInfoParser.endElement(null, ProjectInfo.Fields.GENERAL_AREA, null)
        projectInfoParser.endElement(null, PROJECT, null)
        expected.name = PROJECT_INFO_NAME
        expected.generalArea = "Project General Area"
        Assert.assertEquals(listOf(expected), projectInfoParser.projectInfos)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectInfo has name and specific_area then expect list with matching ProjectInfo`() {
        projectInfoParser.startElement(null, PROJECT, null, null)
        projectInfoParser.startElement(null, NAME, null, null)
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length)
        projectInfoParser.endElement(null, NAME, null)
        projectInfoParser.startElement(
            null, ProjectInfo.Fields.SPECIFIC_AREA, null,
            null
        )
        projectInfoParser.characters(
            "Project Specific Area".toCharArray(), 0,
            "Project Specific Area".length
        )
        projectInfoParser.endElement(null, ProjectInfo.Fields.SPECIFIC_AREA, null)
        projectInfoParser.endElement(null, PROJECT, null)
        expected.name = PROJECT_INFO_NAME
        expected.specificArea = "Project Specific Area"
        Assert.assertEquals(listOf(expected), projectInfoParser.projectInfos)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectInfo has name and description then expect list with matching ProjectInfo`() {
        projectInfoParser.startElement(null, PROJECT, null, null)
        projectInfoParser.startElement(null, NAME, null, null)
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length)
        projectInfoParser.endElement(null, NAME, null)
        projectInfoParser.startElement(
            null, DESCRIPTION, null,
            null
        )
        projectInfoParser.characters(
            "Project Description".toCharArray(), 0,
            "Project Description".length
        )
        projectInfoParser.endElement(null, DESCRIPTION, null)
        projectInfoParser.endElement(null, PROJECT, null)
        expected.name = PROJECT_INFO_NAME
        expected.description = "Project Description"
        Assert.assertEquals(listOf(expected), projectInfoParser.projectInfos)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectInfo has name and home then expect list with matching ProjectInfo`() {
        projectInfoParser.startElement(null, PROJECT, null, null)
        projectInfoParser.startElement(null, NAME, null, null)
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length)
        projectInfoParser.endElement(null, NAME, null)
        projectInfoParser.startElement(
            null, ProjectInfo.Fields.HOME, null,
            null
        )
        projectInfoParser.characters("Project Home".toCharArray(), 0, "Project Home".length)
        projectInfoParser.endElement(null, ProjectInfo.Fields.HOME, null)
        projectInfoParser.endElement(null, PROJECT, null)
        expected.name = PROJECT_INFO_NAME
        expected.home = "Project Home"
        Assert.assertEquals(listOf(expected), projectInfoParser.projectInfos)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectInfo has name and image_url then expect list with matching ProjectInfo`() {
        projectInfoParser.startElement(null, PROJECT, null, null)
        projectInfoParser.startElement(null, NAME, null, null)
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length)
        projectInfoParser.endElement(null, NAME, null)
        projectInfoParser.startElement(
            null, ProjectInfo.Fields.IMAGE_URL, null,
            null
        )
        projectInfoParser.characters(
            "Project Image URL".toCharArray(), 0,
            "Project Image URL".length
        )
        projectInfoParser.endElement(null, ProjectInfo.Fields.IMAGE_URL, null)
        projectInfoParser.endElement(null, PROJECT, null)
        expected.name = PROJECT_INFO_NAME
        expected.imageUrl = "Project Image URL"
        Assert.assertEquals(listOf(expected), projectInfoParser.projectInfos)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectInfo has name and summary then expect list with matching ProjectInfo`() {
        projectInfoParser.startElement(null, PROJECT, null, null)
        projectInfoParser.startElement(null, NAME, null, null)
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length)
        projectInfoParser.endElement(null, NAME, null)
        projectInfoParser.startElement(
            null, ProjectInfo.Fields.SUMMARY, null,
            null
        )
        projectInfoParser.characters(
            "Project Summary".toCharArray(), 0,
            "Project Summary".length
        )
        projectInfoParser.endElement(null, ProjectInfo.Fields.SUMMARY, null)
        projectInfoParser.endElement(null, PROJECT, null)
        expected.name = PROJECT_INFO_NAME
        expected.summary = "Project Summary"
        Assert.assertEquals(listOf(expected), projectInfoParser.projectInfos)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectInfo has name and one platform then expect list with matching ProjectInfo`() {
        projectInfoParser.startElement(null, PROJECT, null, null)
        projectInfoParser.startElement(null, NAME, null, null)
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length)
        projectInfoParser.endElement(null, NAME, null)
        projectInfoParser.startElement(null, ProjectInfo.Fields.PLATFORMS, null, null)
        projectInfoParser.startElement(null, NAME, null, null)
        projectInfoParser.characters("Platform Name".toCharArray(), 0, "Platform Name".length)
        projectInfoParser.endElement(null, NAME, null)
        projectInfoParser.endElement(null, ProjectInfo.Fields.PLATFORMS, null)
        projectInfoParser.endElement(null, PROJECT, null)
        expected.name = PROJECT_INFO_NAME
        expected.platforms = listOf("Platform Name")
        Assert.assertEquals(listOf(expected), projectInfoParser.projectInfos)
    }

    companion object {
        private const val PROJECT_INFO_NAME = "Project Info"
    }
}
