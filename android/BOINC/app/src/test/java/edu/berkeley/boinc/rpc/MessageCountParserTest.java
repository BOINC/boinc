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
import org.xml.sax.SAXException;

import static org.junit.Assert.assertEquals;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class MessageCountParserTest {
    private MessageCountParser messageCountParser;

    @Before
    public void setUp() {
        messageCountParser = new MessageCountParser();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectMinus1() {
        mockStatic(Xml.class);

        assertEquals(-1, MessageCountParser.getSeqno(null));
    }

    @Test
    public void testParse_whenRpcStringIsEmpty_thenExpectMinus1() {
        mockStatic(Xml.class);

        assertEquals(-1, MessageCountParser.getSeqno(""));
    }

    @Test
    public void testParser_whenXmlReplyHasNoContent_thenExpectSeqNoToBeMinus1() throws SAXException {
        messageCountParser.startElement(null, MessageCountParser.REPLY_TAG, null, null);
        messageCountParser.endElement(null, MessageCountParser.REPLY_TAG, null);

        assertEquals(-1, messageCountParser.seqno());
    }

    @Test
    public void testParser_whenXmlReplyHasInvalidSeqNo_thenExpectStringBuilderToBeEmptyAndSeqNoToBeMinus1() throws SAXException {
        messageCountParser.startElement(null, MessageCountParser.REPLY_TAG, null, null);
        messageCountParser.startElement(null, "seqno", null, null);
        messageCountParser.characters("One".toCharArray(), 0, 3);
        messageCountParser.endElement(null, "seqno", null);
        messageCountParser.endElement(null, MessageCountParser.REPLY_TAG, null);

        assertEquals(0, messageCountParser.mCurrentElement.length());
        assertEquals(-1, messageCountParser.seqno());
    }

    @Test
    public void testParser_whenXmlReplyHasSeqNo2_thenExpectStringBuilderToBeEmptyAndSeqNoToBe2()
            throws SAXException {
        messageCountParser.startElement(null, MessageCountParser.REPLY_TAG, null, null);
        messageCountParser.startElement(null, "seqno", null, null);
        messageCountParser.characters("2".toCharArray(), 0, 1);
        messageCountParser.endElement(null, "seqno", null);
        messageCountParser.endElement(null, MessageCountParser.REPLY_TAG, null);

        assertEquals(0, messageCountParser.mCurrentElement.length());
        assertEquals(2, messageCountParser.seqno());
    }
}
