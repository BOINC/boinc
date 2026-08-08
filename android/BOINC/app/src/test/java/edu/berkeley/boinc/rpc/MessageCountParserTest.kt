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
import edu.berkeley.boinc.rpc.MessageCountParser.Companion.getSeqnoOfReply
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
class MessageCountParserTest {
    private lateinit var messageCountParser: MessageCountParser
    @Before
    fun setUp() {
        messageCountParser = MessageCountParser()
    }

    @Test
    fun `When Rpc string is null then expect -1`() {
        PowerMockito.mockStatic(Xml::class.java)
        Assert.assertEquals(-1, getSeqnoOfReply(null).toLong())
    }

    @Test
    fun `When Rpc string is empty then expect -1`() {
        PowerMockito.mockStatic(Xml::class.java)
        Assert.assertEquals(-1, getSeqnoOfReply("").toLong())
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown then expect -1`() {
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertEquals(-1, getSeqnoOfReply("").toLong())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml reply has no content then expect seqno to be -1`() {
        messageCountParser.startElement(null, MessageCountParser.REPLY_TAG, null, null)
        messageCountParser.endElement(null, MessageCountParser.REPLY_TAG, null)
        Assert.assertEquals(-1, messageCountParser.seqno)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml reply has invalid seqno then expect StringBuilder to be empty and seqno to be -1`() {
        PowerMockito.mockStatic(Log::class.java)
        messageCountParser.startElement(null, MessageCountParser.REPLY_TAG, null, null)
        messageCountParser.startElement(null, "seqno", null, null)
        messageCountParser.characters("One".toCharArray(), 0, 3)
        messageCountParser.endElement(null, "seqno", null)
        messageCountParser.endElement(null, MessageCountParser.REPLY_TAG, null)
        Assert.assertEquals(0, messageCountParser.mCurrentElement.length.toLong())
        Assert.assertEquals(-1, messageCountParser.seqno)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml reply has seqno 2 then expect StringBuilder to be empty and seqno to be 2`() {
        messageCountParser.startElement(null, MessageCountParser.REPLY_TAG, null, null)
        messageCountParser.startElement(null, "seqno", null, null)
        messageCountParser.characters("2".toCharArray(), 0, 1)
        messageCountParser.endElement(null, "seqno", null)
        messageCountParser.endElement(null, MessageCountParser.REPLY_TAG, null)
        Assert.assertEquals(0, messageCountParser.mCurrentElement.length.toLong())
        Assert.assertEquals(2, messageCountParser.seqno)
    }
}
