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
import edu.berkeley.boinc.rpc.SimpleReplyParser.Companion.parse
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
class SimpleReplyParserTest {
    private lateinit var simpleReplyParser: SimpleReplyParser
    @Before
    fun setUp() {
        simpleReplyParser = SimpleReplyParser()
    }

    @Test
    fun `When Rpc string is null then expect success to be false`() {
        PowerMockito.mockStatic(Xml::class.java)
        val result = parse(null)
        Assert.assertNotNull(result)
        Assert.assertFalse(result!!.result)
    }

    @Test
    fun `When Rpc string is empty then expect success to be false`() {
        PowerMockito.mockStatic(Xml::class.java)
        val result = parse("")
        Assert.assertNotNull(result)
        Assert.assertFalse(result!!.result)
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
    fun `When Xml Reply has no content then expect success to be false`() {
        simpleReplyParser.startElement(null, MessageCountParser.REPLY_TAG, null, null)
        simpleReplyParser.endElement(null, MessageCountParser.REPLY_TAG, null)
        Assert.assertFalse(simpleReplyParser.result)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Reply has success then expect success to be true`() {
        simpleReplyParser.startElement(null, MessageCountParser.REPLY_TAG, null, null)
        simpleReplyParser.startElement(null, "success", null, null)
        simpleReplyParser.endElement(null, "success", null)
        simpleReplyParser.endElement(null, MessageCountParser.REPLY_TAG, null)
        Assert.assertTrue(simpleReplyParser.result)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Reply has failure then expect success to be false`() {
        simpleReplyParser.startElement(null, MessageCountParser.REPLY_TAG, null, null)
        simpleReplyParser.startElement(null, "failure", null, null)
        simpleReplyParser.endElement(null, "failure", null)
        simpleReplyParser.endElement(null, MessageCountParser.REPLY_TAG, null)
        Assert.assertFalse(simpleReplyParser.result)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Reply has error then expect success to be false and buffer to have error message`() {
        simpleReplyParser.startElement(null, MessageCountParser.REPLY_TAG, null, null)
        simpleReplyParser.startElement(null, "error", null, null)
        simpleReplyParser.characters("Error message".toCharArray(), 0, "Error message".length)
        simpleReplyParser.endElement(null, "error", null)
        simpleReplyParser.endElement(null, MessageCountParser.REPLY_TAG, null)
        Assert.assertFalse(simpleReplyParser.result)
        Assert.assertEquals("Error message", simpleReplyParser.mCurrentElement.toString())
    }
}
