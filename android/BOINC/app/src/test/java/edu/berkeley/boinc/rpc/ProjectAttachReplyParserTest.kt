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
import edu.berkeley.boinc.rpc.ProjectAttachReplyParser.Companion.parse
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
class ProjectAttachReplyParserTest {
    private lateinit var projectAttachReplyParser: ProjectAttachReplyParser
    private lateinit var expected: ProjectAttachReply
    @Before
    fun setUp() {
        projectAttachReplyParser = ProjectAttachReplyParser()
        expected = ProjectAttachReply()
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
    fun `When 'localName' is empty then expect element started`() {
        projectAttachReplyParser.startElement(null, "", null, null)
        Assert.assertTrue(projectAttachReplyParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is project_attach_reply tag then expect element not started`() {
        projectAttachReplyParser.startElement(
            null, ProjectAttachReplyParser.PROJECT_ATTACH_REPLY_TAG,
            null, null
        )
        projectAttachReplyParser.endElement(
            null, ProjectAttachReplyParser.PROJECT_ATTACH_REPLY_TAG,
            null
        )
        Assert.assertFalse(projectAttachReplyParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectAttachReply has error_num then expect matching ProjectAttachReply`() {
        projectAttachReplyParser.startElement(
            null, ProjectAttachReplyParser.PROJECT_ATTACH_REPLY_TAG,
            null, null
        )
        projectAttachReplyParser.startElement(
            null, ProjectAttachReplyParser.ERROR_NUM_TAG,
            null, null
        )
        projectAttachReplyParser.characters("1".toCharArray(), 0, 1)
        projectAttachReplyParser.endElement(
            null, ProjectAttachReplyParser.ERROR_NUM_TAG,
            null
        )
        projectAttachReplyParser.endElement(
            null, ProjectAttachReplyParser.PROJECT_ATTACH_REPLY_TAG,
            null
        )
        expected.errorNum = 1
        Assert.assertEquals(expected, projectAttachReplyParser.projectAttachReply)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectAttachReply has message then expect matching ProjectAttachReply`() {
        projectAttachReplyParser.startElement(
            null, ProjectAttachReplyParser.PROJECT_ATTACH_REPLY_TAG,
            null, null
        )
        projectAttachReplyParser.startElement(null, MESSAGE, null, null)
        projectAttachReplyParser.characters("Message".toCharArray(), 0, 7)
        projectAttachReplyParser.endElement(null, MESSAGE, null)
        projectAttachReplyParser.endElement(
            null, ProjectAttachReplyParser.PROJECT_ATTACH_REPLY_TAG,
            null
        )
        expected.messages.add("Message")
        Assert.assertEquals(expected, projectAttachReplyParser.projectAttachReply)
    }
}
