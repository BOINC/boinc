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
import org.xml.sax.SAXException;

import java.util.Collections;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest({Xml.class, Log.class})
public class NoticesParserTest {
    private NoticesParser noticesParser;
    private Notice expected;

    @Before
    public void setUp() {
        noticesParser = new NoticesParser();
        expected = new Notice();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectNullPointerException() {
        mockStatic(Xml.class);

        assertThrows(NullPointerException.class, () -> NoticesParser.parse(null));
    }

    @Test
    public void testParse_whenRpcStringIsEmpty_thenExpectEmptyList() {
        mockStatic(Xml.class);

        assertTrue(NoticesParser.parse("").isEmpty());
    }

    @Test
    public void testParser_whenOnlyStartElementIsRun_thenExpectElementStarted() throws SAXException {
        noticesParser.startElement(null, "", null, null);

        assertTrue(noticesParser.mElementStarted);
    }

    @Test
    public void testParser_whenBothStartElementAndEndElementAreRun_thenExpectElementNotStarted()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        assertFalse(noticesParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectEmptyList() throws SAXException {
        noticesParser.startElement(null, "", null, null);

        assertTrue(noticesParser.getNotices().isEmpty());
    }

    @Test
    public void testParser_whenXmlNoticeHasNoElements_thenExpectEmptyList()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        assertTrue(noticesParser.getNotices().isEmpty());
    }

    @Test
    public void testParser_whenXmlNoticeHasInvalidSeqno_thenExpectEmptyList() throws SAXException {
        mockStatic(Log.class);

        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.seqno, null, null);
        noticesParser.characters("One".toCharArray(), 0, 3);
        noticesParser.endElement(null, Notice.Fields.seqno, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        assertTrue(noticesParser.getNotices().isEmpty());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqno_thenExpectListWithMatchingNotice() throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.seqno, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.seqno, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.seqno = 1;

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndTitle_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.seqno, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.seqno, null);
        noticesParser.startElement(null, Notice.Fields.title, null, null);
        noticesParser.characters("Notice".toCharArray(), 0, 6);
        noticesParser.endElement(null, Notice.Fields.title, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.seqno = 1;
        expected.title = "Notice";

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndDescription_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.seqno, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.seqno, null);
        noticesParser.startElement(null, Notice.Fields.description, null, null);
        noticesParser.characters("This is a notice.".toCharArray(), 0, "This is a notice.".length());
        noticesParser.endElement(null, Notice.Fields.description, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.seqno = 1;
        expected.description = "This is a notice.";

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndCreateTime_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.seqno, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.seqno, null);
        noticesParser.startElement(null, Notice.Fields.create_time, null, null);
        noticesParser.characters("1.5".toCharArray(), 0, 3);
        noticesParser.endElement(null, Notice.Fields.create_time, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.seqno = 1;
        expected.create_time = 1.5;

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndArrivalTime_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.seqno, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.seqno, null);
        noticesParser.startElement(null, Notice.Fields.arrival_time, null, null);
        noticesParser.characters("1.5".toCharArray(), 0, 3);
        noticesParser.endElement(null, Notice.Fields.arrival_time, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.seqno = 1;
        expected.arrival_time = 1.5;

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndCategoryUnknown_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.seqno, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.seqno, null);
        noticesParser.startElement(null, Notice.Fields.category, null, null);
        noticesParser.characters("unknown".toCharArray(), 0, 7);
        noticesParser.endElement(null, Notice.Fields.category, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.seqno = 1;
        expected.category = "unknown";

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndCategoryServer_thenExpectListWithMatchingServerNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.seqno, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.seqno, null);
        noticesParser.startElement(null, Notice.Fields.category, null, null);
        noticesParser.characters("server".toCharArray(), 0, 6);
        noticesParser.endElement(null, Notice.Fields.category, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.seqno = 1;
        expected.category = "server";
        expected.isServerNotice = true;

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndCategoryScheduler_thenExpectListWithMatchingServerNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.seqno, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.seqno, null);
        noticesParser.startElement(null, Notice.Fields.category, null, null);
        noticesParser.characters("scheduler".toCharArray(), 0, 9);
        noticesParser.endElement(null, Notice.Fields.category, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.seqno = 1;
        expected.category = "scheduler";
        expected.isServerNotice = true;

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndCategoryClient_thenExpectListWithMatchingClientNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.seqno, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.seqno, null);
        noticesParser.startElement(null, Notice.Fields.category, null, null);
        noticesParser.characters("client".toCharArray(), 0, 6);
        noticesParser.endElement(null, Notice.Fields.category, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.seqno = 1;
        expected.category = "client";
        expected.isClientNotice = true;

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndLink_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.seqno, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.seqno, null);
        noticesParser.startElement(null, Notice.Fields.link, null, null);
        noticesParser.characters("Link".toCharArray(), 0, 4);
        noticesParser.endElement(null, Notice.Fields.link, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.seqno = 1;
        expected.link = "Link";

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndProjectName_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.seqno, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.seqno, null);
        noticesParser.startElement(null, Notice.Fields.project_name, null, null);
        noticesParser.characters("Project Name".toCharArray(), 0, "Project Name".length());
        noticesParser.endElement(null, Notice.Fields.project_name, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.seqno = 1;
        expected.project_name = "Project Name";

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }
}
