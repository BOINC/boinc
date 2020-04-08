/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2020 University of California
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
package edu.berkeley.boinc.rpc;

import android.util.Xml;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.xml.sax.ContentHandler;
import org.xml.sax.SAXException;

import static org.junit.Assert.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.doThrow;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class SimpleReplyParserTest {
    private SimpleReplyParser simpleReplyParser;

    @Before
    public void setUp() {
        simpleReplyParser = new SimpleReplyParser();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectSuccessToBeFalse() {
        mockStatic(Xml.class);

        SimpleReplyParser result = SimpleReplyParser.parse(null);

        assertNotNull(result);
        assertFalse(result.getResult());
    }

    @Test
    public void testParse_whenRpcStringIsEmpty_thenExpectSuccessToBeFalse() {
        mockStatic(Xml.class);

        SimpleReplyParser result = SimpleReplyParser.parse("");

        assertNotNull(result);
        assertFalse(result.getResult());
    }

    @Test
    public void testParse_whenSAXExceptionIsThrown_thenExpectNull() throws Exception {
        mockStatic(Xml.class);

        doThrow(new SAXException()).when(Xml.class, "parse", anyString(), any(ContentHandler.class));

        assertNull(SimpleReplyParser.parse(""));
    }

    @Test
    public void testParser_whenXmlReplyHasNoContent_thenExpectSuccessToBeFalse() throws SAXException {
        simpleReplyParser.startElement(null, MessageCountParser.REPLY_TAG, null, null);
        simpleReplyParser.endElement(null, MessageCountParser.REPLY_TAG, null);

        assertFalse(simpleReplyParser.getResult());
    }

    @Test
    public void testParser_whenXmlReplyHasSuccess_thenExpectSuccessToBeTrue() throws SAXException {
        simpleReplyParser.startElement(null, MessageCountParser.REPLY_TAG, null, null);
        simpleReplyParser.startElement(null, "success", null, null);
        simpleReplyParser.endElement(null, "success", null);
        simpleReplyParser.endElement(null, MessageCountParser.REPLY_TAG, null);

        assertTrue(simpleReplyParser.getResult());
    }

    @Test
    public void testParser_whenXmlReplyHasFailure_thenExpectSuccessToBeFalse() throws SAXException {
        simpleReplyParser.startElement(null, MessageCountParser.REPLY_TAG, null, null);
        simpleReplyParser.startElement(null, "failure", null, null);
        simpleReplyParser.endElement(null, "failure", null);
        simpleReplyParser.endElement(null, MessageCountParser.REPLY_TAG, null);

        assertFalse(simpleReplyParser.getResult());
    }

    @Test
    public void testParser_whenXmlReplyHasError_thenExpectSuccessToBeFalseAndBufferToHaveErrorMessage()
            throws SAXException {
        simpleReplyParser.startElement(null, MessageCountParser.REPLY_TAG, null, null);
        simpleReplyParser.startElement(null, "error", null, null);
        simpleReplyParser.characters("Error message".toCharArray(), 0, "Error message".length());
        simpleReplyParser.endElement(null, "error", null);
        simpleReplyParser.endElement(null, MessageCountParser.REPLY_TAG, null);

        assertFalse(simpleReplyParser.getResult());
        assertEquals("Error message", simpleReplyParser.mCurrentElement.toString());
    }
}
