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
import edu.berkeley.boinc.rpc.NoticesParser.Companion.parse
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
class NoticesParserTest {
    private lateinit var noticesParser: NoticesParser
    private lateinit var expected: Notice
    @Before
    fun setUp() {
        noticesParser = NoticesParser()
        expected = Notice()
    }

    @Test
    fun `When Rpc string is empty then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        Assert.assertTrue(parse("").isEmpty())
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown hen expect empty list`() {
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
    fun `When only start element is run then expect element started`() {
        noticesParser.startElement(null, "", null, null)
        Assert.assertTrue(noticesParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When both start element and end element are run then expect element not started`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        Assert.assertFalse(noticesParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect empty list`() {
        noticesParser.startElement(null, "", null, null)
        Assert.assertTrue(noticesParser.notices.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has no elements then expect empty list`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        Assert.assertTrue(noticesParser.notices.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has invalid seqno then expect empty list`() {
        PowerMockito.mockStatic(Log::class.java)
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null)
        noticesParser.characters("One".toCharArray(), 0, 3)
        noticesParser.endElement(null, Notice.Fields.SEQNO, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        Assert.assertTrue(noticesParser.notices.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has seqno then expect list with matching Notice`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null)
        noticesParser.characters("1".toCharArray(), 0, 1)
        noticesParser.endElement(null, Notice.Fields.SEQNO, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        expected.seqno = 1
        Assert.assertEquals(listOf(expected), noticesParser.notices)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has seqno and title then expect list with matching Notice`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null)
        noticesParser.characters("1".toCharArray(), 0, 1)
        noticesParser.endElement(null, Notice.Fields.SEQNO, null)
        noticesParser.startElement(null, Notice.Fields.TITLE, null, null)
        noticesParser.characters("Notice".toCharArray(), 0, 6)
        noticesParser.endElement(null, Notice.Fields.TITLE, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        expected.seqno = 1
        expected.title = "Notice"
        Assert.assertEquals(listOf(expected), noticesParser.notices)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has seqno and description then expect list with matching Notice`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null)
        noticesParser.characters("1".toCharArray(), 0, 1)
        noticesParser.endElement(null, Notice.Fields.SEQNO, null)
        noticesParser.startElement(null, DESCRIPTION, null, null)
        noticesParser.characters("This is a notice.".toCharArray(), 0, "This is a notice.".length)
        noticesParser.endElement(null, DESCRIPTION, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        expected.seqno = 1
        expected.description = "This is a notice."
        Assert.assertEquals(listOf(expected), noticesParser.notices)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has seqno and create_time then expect list with matching Notice`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null)
        noticesParser.characters("1".toCharArray(), 0, 1)
        noticesParser.endElement(null, Notice.Fields.SEQNO, null)
        noticesParser.startElement(null, Notice.Fields.CREATE_TIME, null, null)
        noticesParser.characters("1.5".toCharArray(), 0, 3)
        noticesParser.endElement(null, Notice.Fields.CREATE_TIME, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        expected.seqno = 1
        expected.createTime = 1.5
        Assert.assertEquals(listOf(expected), noticesParser.notices)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has seqno and arrival_time then expect list with matching Notice`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null)
        noticesParser.characters("1".toCharArray(), 0, 1)
        noticesParser.endElement(null, Notice.Fields.SEQNO, null)
        noticesParser.startElement(null, Notice.Fields.ARRIVAL_TIME, null, null)
        noticesParser.characters("1.5".toCharArray(), 0, 3)
        noticesParser.endElement(null, Notice.Fields.ARRIVAL_TIME, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        expected.seqno = 1
        expected.arrivalTime = 1.5
        Assert.assertEquals(listOf(expected), noticesParser.notices)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has seqno and category Unknown then expect list with matching Notice`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null)
        noticesParser.characters("1".toCharArray(), 0, 1)
        noticesParser.endElement(null, Notice.Fields.SEQNO, null)
        noticesParser.startElement(null, Notice.Fields.Category, null, null)
        noticesParser.characters("unknown".toCharArray(), 0, 7)
        noticesParser.endElement(null, Notice.Fields.Category, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        expected.seqno = 1
        expected.category = "unknown"
        Assert.assertEquals(listOf(expected), noticesParser.notices)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has seqno and category Server then expect list with matching server Notice`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null)
        noticesParser.characters("1".toCharArray(), 0, 1)
        noticesParser.endElement(null, Notice.Fields.SEQNO, null)
        noticesParser.startElement(null, Notice.Fields.Category, null, null)
        noticesParser.characters("server".toCharArray(), 0, 6)
        noticesParser.endElement(null, Notice.Fields.Category, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        expected.seqno = 1
        expected.category = "server"
        expected.isServerNotice = true
        Assert.assertEquals(listOf(expected), noticesParser.notices)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has seqno and category Scheduler then expect list with matching server Notice`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null)
        noticesParser.characters("1".toCharArray(), 0, 1)
        noticesParser.endElement(null, Notice.Fields.SEQNO, null)
        noticesParser.startElement(null, Notice.Fields.Category, null, null)
        noticesParser.characters("scheduler".toCharArray(), 0, 9)
        noticesParser.endElement(null, Notice.Fields.Category, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        expected.seqno = 1
        expected.category = "scheduler"
        expected.isServerNotice = true
        Assert.assertEquals(listOf(expected), noticesParser.notices)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has seqno and category Client then expect list with matching client Notice`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null)
        noticesParser.characters("1".toCharArray(), 0, 1)
        noticesParser.endElement(null, Notice.Fields.SEQNO, null)
        noticesParser.startElement(null, Notice.Fields.Category, null, null)
        noticesParser.characters("client".toCharArray(), 0, 6)
        noticesParser.endElement(null, Notice.Fields.Category, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        expected.seqno = 1
        expected.category = "client"
        expected.isClientNotice = true
        Assert.assertEquals(listOf(expected), noticesParser.notices)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has seqno and link then expect list with matching Notice`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null)
        noticesParser.characters("1".toCharArray(), 0, 1)
        noticesParser.endElement(null, Notice.Fields.SEQNO, null)
        noticesParser.startElement(null, Notice.Fields.LINK, null, null)
        noticesParser.characters("Link".toCharArray(), 0, 4)
        noticesParser.endElement(null, Notice.Fields.LINK, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        expected.seqno = 1
        expected.link = "Link"
        Assert.assertEquals(listOf(expected), noticesParser.notices)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Notice has seqno and project_name then expect list with matching Notice`() {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null)
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null)
        noticesParser.characters("1".toCharArray(), 0, 1)
        noticesParser.endElement(null, Notice.Fields.SEQNO, null)
        noticesParser.startElement(null, PROJECT_NAME, null, null)
        noticesParser.characters("Project Name".toCharArray(), 0, "Project Name".length)
        noticesParser.endElement(null, PROJECT_NAME, null)
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null)
        expected.seqno = 1
        expected.projectName = "Project Name"
        Assert.assertEquals(listOf(expected), noticesParser.notices)
    }
}
