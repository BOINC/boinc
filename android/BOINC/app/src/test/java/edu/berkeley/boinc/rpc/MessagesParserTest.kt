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
import edu.berkeley.boinc.rpc.MessagesParser.Companion.parse
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
@PrepareForTest(Xml::class, Log::class)
class MessagesParserTest {
    private lateinit var messagesParser: MessagesParser
    private lateinit var expected: Message
    @Before
    fun setUp() {
        messagesParser = MessagesParser()
        expected = Message()
    }

    @Test
    fun `When Rpc result is empty then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        Assert.assertTrue(parse("").isEmpty())
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
        messagesParser.startElement(null, "", null, null)
        Assert.assertTrue(messagesParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When both start element and end element are run then expect element not started`() {
        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null)
        messagesParser.endElement(null, MessagesParser.MESSAGE, null)
        Assert.assertFalse(messagesParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect empty list`() {
        messagesParser.startElement(null, "", null, null)
        Assert.assertTrue(messagesParser.messages.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Message has no elements then expect empty list`() {
        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null)
        messagesParser.endElement(null, MessagesParser.MESSAGE, null)
        Assert.assertTrue(messagesParser.messages.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Message has invalid seqno then expect empty list`() {
        PowerMockito.mockStatic(Log::class.java)
        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null)
        messagesParser.startElement(null, SEQNO, null, null)
        messagesParser.characters("One".toCharArray(), 0, 3)
        messagesParser.endElement(null, SEQNO, null)
        messagesParser.endElement(null, MessagesParser.MESSAGE, null)
        Assert.assertTrue(messagesParser.messages.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Message has seqno then expect list with matching Message`() {
        PowerMockito.mockStatic(Log::class.java)
        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null)
        messagesParser.startElement(null, SEQNO, null, null)
        messagesParser.characters("1".toCharArray(), 0, 1)
        messagesParser.endElement(null, SEQNO, null)
        messagesParser.endElement(null, MessagesParser.MESSAGE, null)
        expected.seqno = 1
        Assert.assertEquals(listOf(expected), messagesParser.messages)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Message has seqno without closing tag then expect empty list`() {
        PowerMockito.mockStatic(Log::class.java)
        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null)
        messagesParser.startElement(null, SEQNO, null, null)
        messagesParser.characters("1".toCharArray(), 0, 1)
        messagesParser.endElement(null, "", null)
        messagesParser.endElement(null, MessagesParser.MESSAGE, null)
        Assert.assertTrue(messagesParser.messages.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Message has seqno and body then expect list with matching Message`() {
        PowerMockito.mockStatic(Log::class.java)
        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null)
        messagesParser.startElement(null, SEQNO, null, null)
        messagesParser.characters("1".toCharArray(), 0, 1)
        messagesParser.endElement(null, SEQNO, null)
        messagesParser.startElement(null, Message.Fields.BODY, null, null)
        messagesParser.characters("Body".toCharArray(), 0, 4)
        messagesParser.endElement(null, Message.Fields.BODY, null)
        messagesParser.endElement(null, MessagesParser.MESSAGE, null)
        expected.seqno = 1
        expected.body = "Body"
        Assert.assertEquals(listOf(expected), messagesParser.messages)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Message has seqno and priority then expect list with matching Message`() {
        PowerMockito.mockStatic(Log::class.java)
        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null)
        messagesParser.startElement(null, SEQNO, null, null)
        messagesParser.characters("1".toCharArray(), 0, 1)
        messagesParser.endElement(null, SEQNO, null)
        messagesParser.startElement(null, Message.Fields.PRIORITY, null, null)
        messagesParser.characters("1".toCharArray(), 0, 1)
        messagesParser.endElement(null, Message.Fields.PRIORITY, null)
        messagesParser.endElement(null, MessagesParser.MESSAGE, null)
        expected.seqno = 1
        expected.priority = 1
        Assert.assertEquals(listOf(expected), messagesParser.messages)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Message has seqno and project then expect list with matching Message`() {
        PowerMockito.mockStatic(Log::class.java)
        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null)
        messagesParser.startElement(null, SEQNO, null, null)
        messagesParser.characters("1".toCharArray(), 0, 1)
        messagesParser.endElement(null, SEQNO, null)
        messagesParser.startElement(null, PROJECT, null, null)
        messagesParser.characters("Project".toCharArray(), 0, 1)
        messagesParser.endElement(null, PROJECT, null)
        messagesParser.endElement(null, MessagesParser.MESSAGE, null)
        expected.seqno = 1
        expected.project = "P"
        Assert.assertEquals(listOf(expected), messagesParser.messages)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Message has seqno and timestamp then expect list with matching Message`() {
        PowerMockito.mockStatic(Log::class.java)
        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null)
        messagesParser.startElement(null, SEQNO, null, null)
        messagesParser.characters("1".toCharArray(), 0, 1)
        messagesParser.endElement(null, SEQNO, null)
        messagesParser.startElement(null, Message.Fields.TIMESTAMP, null, null)
        messagesParser.characters("1.5".toCharArray(), 0, 1)
        messagesParser.endElement(null, Message.Fields.TIMESTAMP, null)
        messagesParser.endElement(null, MessagesParser.MESSAGE, null)
        expected.seqno = 1
        expected.timestamp = 1
        Assert.assertEquals(listOf(expected), messagesParser.messages)
    }
}
