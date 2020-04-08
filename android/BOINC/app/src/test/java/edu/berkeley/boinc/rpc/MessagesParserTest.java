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

import android.util.Log;
import android.util.Xml;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.xml.sax.ContentHandler;
import org.xml.sax.SAXException;

import java.util.Collections;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.doThrow;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest({Xml.class, Log.class})
public class MessagesParserTest {
    private MessagesParser messagesParser;
    private Message expected;

    @Before
    public void setUp() {
        messagesParser = new MessagesParser();
        expected = new Message();
    }

    @Test(expected = IllegalArgumentException.class)
    public void testParse_whenRpcResultIsNull_thenExpectIllegalArgumentException() {
        mockStatic(Xml.class);

        MessagesParser.parse(null);
    }

    @Test
    public void testParse_whenRpcResultIsEmpty_thenExpectEmptyList() {
        mockStatic(Xml.class);

        assertTrue(MessagesParser.parse("").isEmpty());
    }

    @Test
    public void testParse_whenSAXExceptionIsThrown_thenExpectEmptyList() throws Exception {
        mockStatic(Xml.class);

        doThrow(new SAXException()).when(Xml.class, "parse", anyString(), any(ContentHandler.class));

        assertTrue(MessagesParser.parse("").isEmpty());
    }

    @Test
    public void testParser_whenOnlyStartElementIsRun_thenExpectElementStarted() throws SAXException {
        messagesParser.startElement(null, "", null, null);

        assertTrue(messagesParser.mElementStarted);
    }

    @Test
    public void testParser_whenBothStartElementAndEndElementAreRun_thenExpectElementNotStarted()
            throws SAXException {
        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null);
        messagesParser.endElement(null, MessagesParser.MESSAGE, null);

        assertFalse(messagesParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectEmptyList() throws SAXException {
        messagesParser.startElement(null, "", null, null);

        assertTrue(messagesParser.getMessages().isEmpty());
    }

    @Test
    public void testParser_whenXmlMessageHasNoElements_thenExpectEmptyList() throws SAXException {
        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null);
        messagesParser.endElement(null, MessagesParser.MESSAGE, null);

        assertTrue(messagesParser.getMessages().isEmpty());
    }

    @Test
    public void testParser_whenXmlMessageHasInvalidSeqno_thenExpectEmptyList() throws SAXException {
        mockStatic(Log.class);

        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null);
        messagesParser.startElement(null, RPCCommonTags.SEQNO, null, null);
        messagesParser.characters("One".toCharArray(), 0, 3);
        messagesParser.endElement(null, RPCCommonTags.SEQNO, null);
        messagesParser.endElement(null, MessagesParser.MESSAGE, null);

        assertTrue(messagesParser.getMessages().isEmpty());
    }

    @Test
    public void testParser_whenXmlMessageHasSeqno_thenExpectListWithMatchingMessage() throws SAXException {
        mockStatic(Log.class);

        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null);
        messagesParser.startElement(null, RPCCommonTags.SEQNO, null, null);
        messagesParser.characters("1".toCharArray(), 0, 1);
        messagesParser.endElement(null, RPCCommonTags.SEQNO, null);
        messagesParser.endElement(null, MessagesParser.MESSAGE, null);

        expected.setSeqno(1);

        assertEquals(Collections.singletonList(expected), messagesParser.getMessages());
    }

    @Test
    public void testParser_whenXmlMessageHasSeqnoWithoutClosingTag_thenExpectEmptyList()
            throws SAXException {
        mockStatic(Log.class);

        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null);
        messagesParser.startElement(null, RPCCommonTags.SEQNO, null, null);
        messagesParser.characters("1".toCharArray(), 0, 1);
        messagesParser.endElement(null, "", null);
        messagesParser.endElement(null, MessagesParser.MESSAGE, null);

        assertTrue(messagesParser.getMessages().isEmpty());
    }

    @Test
    public void testParser_whenXmlMessageHasSeqnoAndBody_thenExpectListWithMatchingMessage()
            throws SAXException {
        mockStatic(Log.class);

        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null);
        messagesParser.startElement(null, RPCCommonTags.SEQNO, null, null);
        messagesParser.characters("1".toCharArray(), 0, 1);
        messagesParser.endElement(null, RPCCommonTags.SEQNO, null);
        messagesParser.startElement(null, Message.Fields.BODY, null, null);
        messagesParser.characters("Body".toCharArray(), 0, 4);
        messagesParser.endElement(null, Message.Fields.BODY, null);
        messagesParser.endElement(null, MessagesParser.MESSAGE, null);

        expected.setSeqno(1);
        expected.setBody("Body");

        assertEquals(Collections.singletonList(expected), messagesParser.getMessages());
    }

    @Test
    public void testParser_whenXmlMessageHasSeqnoAndPriority_thenExpectListWithMatchingMessage()
            throws SAXException {
        mockStatic(Log.class);

        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null);
        messagesParser.startElement(null, RPCCommonTags.SEQNO, null, null);
        messagesParser.characters("1".toCharArray(), 0, 1);
        messagesParser.endElement(null, RPCCommonTags.SEQNO, null);
        messagesParser.startElement(null, Message.Fields.PRIORITY, null, null);
        messagesParser.characters("1".toCharArray(), 0, 1);
        messagesParser.endElement(null, Message.Fields.PRIORITY, null);
        messagesParser.endElement(null, MessagesParser.MESSAGE, null);

        expected.setSeqno(1);
        expected.setPriority(1);

        assertEquals(Collections.singletonList(expected), messagesParser.getMessages());
    }

    @Test
    public void testParser_whenXmlMessageHasSeqnoAndProject_thenExpectListWithMatchingMessage()
            throws SAXException {
        mockStatic(Log.class);

        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null);
        messagesParser.startElement(null, RPCCommonTags.SEQNO, null, null);
        messagesParser.characters("1".toCharArray(), 0, 1);
        messagesParser.endElement(null, RPCCommonTags.SEQNO, null);
        messagesParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        messagesParser.characters("Project".toCharArray(), 0, 1);
        messagesParser.endElement(null, RPCCommonTags.PROJECT, null);
        messagesParser.endElement(null, MessagesParser.MESSAGE, null);

        expected.setSeqno(1);
        expected.setProject("Project");

        assertEquals(Collections.singletonList(expected), messagesParser.getMessages());
    }

    @Test
    public void testParser_whenXmlMessageHasSeqnoAndTimestamp_thenExpectListWithMatchingMessage()
            throws SAXException {
        mockStatic(Log.class);

        messagesParser.startElement(null, MessagesParser.MESSAGE, null, null);
        messagesParser.startElement(null, RPCCommonTags.SEQNO, null, null);
        messagesParser.characters("1".toCharArray(), 0, 1);
        messagesParser.endElement(null, RPCCommonTags.SEQNO, null);
        messagesParser.startElement(null, Message.Fields.TIMESTAMP, null, null);
        messagesParser.characters("1.5".toCharArray(), 0, 1);
        messagesParser.endElement(null, Message.Fields.TIMESTAMP, null);
        messagesParser.endElement(null, MessagesParser.MESSAGE, null);

        expected.setSeqno(1);
        expected.setTimestamp(1);

        assertEquals(Collections.singletonList(expected), messagesParser.getMessages());
    }
}
