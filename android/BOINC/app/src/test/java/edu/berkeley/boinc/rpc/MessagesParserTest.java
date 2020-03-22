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

import org.junit.Test;

import java.util.Collections;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;

public class MessagesParserTest {
    @Test
    public void testParse_whenRpcResultIsNull_thenExpectNullPointerException() {
        assertThrows(NullPointerException.class, () -> MessagesParser.parse(null));
    }

    @Test
    public void testParse_whenRpcResultIsEmpty_thenExpectEmptyList() {
        assertTrue(MessagesParser.parse("").isEmpty());
    }

    @Test
    public void testParse_whenRpcResultHasOnlyOpeningAndClosingMsgsTags_thenExpectEmptyList() {
        assertTrue(MessagesParser.parse("<msgs></msgs>").isEmpty());
    }

    @Test
    public void testParse_whenRpcResultHasOneMessage_thenExpectListWithMatchingMessage() {
        final Message expectedMessage = new Message();
        expectedMessage.body = "Body";
        expectedMessage.timestamp = 10;
        expectedMessage.seqno = 1;
        expectedMessage.priority = 1;
        expectedMessage.project = "Project";

        final String input = "<msgs><msg><body>Body</body><time>10</time><seqno>1</seqno>" +
                             "<pri>1</pri><project>Project</project></msg></msgs>";

        assertEquals(Collections.singletonList(expectedMessage), MessagesParser.parse(input));
    }

    @Test
    public void testParse_whenRpcResultHasOneMessageWithLeadingWhitespace_thenExpectListWithMatchingMessage() {
        final Message expectedMessage = new Message();
        expectedMessage.body = "Body";
        expectedMessage.timestamp = 10;
        expectedMessage.seqno = 1;
        expectedMessage.priority = 1;
        expectedMessage.project = "Project";

        final String input = "       <msgs><msg><body>Body</body><time>10</time><seqno>1</seqno>" +
                             "<pri>1</pri><project>Project</project></msg></msgs>";

        assertEquals(Collections.singletonList(expectedMessage), MessagesParser.parse(input));
    }

    @Test
    public void testParse_whenRpcResultHasOneMessageWithUnknownTag_thenExpectListWithMatchingMessage() {
        final Message expectedMessage = new Message();
        expectedMessage.body = "Body";
        expectedMessage.timestamp = 10;
        expectedMessage.seqno = 1;
        expectedMessage.priority = 1;
        expectedMessage.project = "Project";

        final String input = "<msgs><msg><tag></tag><body>Body</body><time>10</time><seqno>1</seqno>"
                             + "<pri>1</pri><project>Project</project></msg></msgs>";

        assertEquals(Collections.singletonList(expectedMessage), MessagesParser.parse(input));
    }

    @Test
    public void testParse_whenRpcResultHasOneMessageWithInvalidPriority_thenExpectEmptyList() {
        final String input = "<msgs><msg><body>Body</body><time>10</time><seqno>1</seqno>"
                             + "<pri>One</pri><project>Project</project></msg></msgs>";

        assertTrue(MessagesParser.parse(input).isEmpty());
    }
}