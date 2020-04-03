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

import kotlin.UninitializedPropertyAccessException;

import static org.junit.Assert.*;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class ProjectAttachReplyParserTest {
    private ProjectAttachReplyParser projectAttachReplyParser;
    private ProjectAttachReply expected;

    @Before
    public void setUp() {
        projectAttachReplyParser = new ProjectAttachReplyParser();
        expected = new ProjectAttachReply();
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParse_whenRpcStringIsNull_thenExpectUninitializedPropertyAccessException() {
        mockStatic(Xml.class);

        ProjectAttachReplyParser.parse(null);
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParse_whenRpcStringIsEmpty_thenExpectUninitializedPropertyAccessException() {
        mockStatic(Xml.class);

        ProjectAttachReplyParser.parse("");
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectElementStarted() throws SAXException {
        projectAttachReplyParser.startElement(null, "", null, null);

        assertTrue(projectAttachReplyParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsProjectAttachReplyTag_thenExpectElementNotStarted()
            throws SAXException {
        projectAttachReplyParser.startElement(null, ProjectAttachReplyParser.PROJECT_ATTACH_REPLY_TAG,
                                              null, null);
        projectAttachReplyParser.endElement(null, ProjectAttachReplyParser.PROJECT_ATTACH_REPLY_TAG,
                                            null);

        assertFalse(projectAttachReplyParser.mElementStarted);
    }

    @Test
    public void testParser_whenXmlProjectAttachReplyHasErrorNum_thenExpectMatchingProjectAttachReply()
            throws SAXException {
        projectAttachReplyParser.startElement(null, ProjectAttachReplyParser.PROJECT_ATTACH_REPLY_TAG,
                                              null, null);
        projectAttachReplyParser.startElement(null, ProjectAttachReplyParser.ERROR_NUM_TAG,
                                              null, null);
        projectAttachReplyParser.characters("1".toCharArray(), 0, 1);
        projectAttachReplyParser.endElement(null, ProjectAttachReplyParser.ERROR_NUM_TAG,
                                            null);
        projectAttachReplyParser.endElement(null, ProjectAttachReplyParser.PROJECT_ATTACH_REPLY_TAG,
                                            null);

        expected.setErrorNum(1);

        assertEquals(expected, projectAttachReplyParser.getProjectAttachReply());
    }

    @Test
    public void testParser_whenXmlProjectAttachReplyHasMessage_thenExpectMatchingProjectAttachReply()
            throws SAXException {
        projectAttachReplyParser.startElement(null, ProjectAttachReplyParser.PROJECT_ATTACH_REPLY_TAG,
                                              null, null);
        projectAttachReplyParser.startElement(null, ProjectAttachReplyParser.MESSAGE_TAG,
                                              null, null);
        projectAttachReplyParser.characters("Message".toCharArray(), 0, 7);
        projectAttachReplyParser.endElement(null, ProjectAttachReplyParser.MESSAGE_TAG,
                                            null);
        projectAttachReplyParser.endElement(null, ProjectAttachReplyParser.PROJECT_ATTACH_REPLY_TAG,
                                            null);

        expected.getMessages().add("Message");

        assertEquals(expected, projectAttachReplyParser.getProjectAttachReply());
    }
}
