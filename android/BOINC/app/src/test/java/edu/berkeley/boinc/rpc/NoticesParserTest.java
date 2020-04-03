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

    @Test(expected = IllegalArgumentException.class)
    public void testParse_whenRpcStringIsNull_thenExpectNullPointerException() {
        mockStatic(Xml.class);

        NoticesParser.parse(null);
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
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null);
        noticesParser.characters("One".toCharArray(), 0, 3);
        noticesParser.endElement(null, Notice.Fields.SEQNO, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        assertTrue(noticesParser.getNotices().isEmpty());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqno_thenExpectListWithMatchingNotice() throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.SEQNO, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.setSeqno(1);

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndTitle_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.SEQNO, null);
        noticesParser.startElement(null, Notice.Fields.TITLE, null, null);
        noticesParser.characters("Notice".toCharArray(), 0, 6);
        noticesParser.endElement(null, Notice.Fields.TITLE, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.setSeqno(1);
        expected.setTitle("Notice");

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndDescription_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.SEQNO, null);
        noticesParser.startElement(null, RPCCommonTags.DESCRIPTION, null, null);
        noticesParser.characters("This is a notice.".toCharArray(), 0, "This is a notice.".length());
        noticesParser.endElement(null, RPCCommonTags.DESCRIPTION, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.setSeqno(1);
        expected.setDescription("This is a notice.");

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndCreateTime_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.SEQNO, null);
        noticesParser.startElement(null, Notice.Fields.CREATE_TIME, null, null);
        noticesParser.characters("1.5".toCharArray(), 0, 3);
        noticesParser.endElement(null, Notice.Fields.CREATE_TIME, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.setSeqno(1);
        expected.setCreateTime(1.5);

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndArrivalTime_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.SEQNO, null);
        noticesParser.startElement(null, Notice.Fields.ARRIVAL_TIME, null, null);
        noticesParser.characters("1.5".toCharArray(), 0, 3);
        noticesParser.endElement(null, Notice.Fields.ARRIVAL_TIME, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.setSeqno(1);
        expected.setArrivalTime(1.5);

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndCategoryUnknown_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.SEQNO, null);
        noticesParser.startElement(null, Notice.Fields.CATEGORY, null, null);
        noticesParser.characters("unknown".toCharArray(), 0, 7);
        noticesParser.endElement(null, Notice.Fields.CATEGORY, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.setSeqno(1);
        expected.setCategory("unknown");

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndCategoryServer_thenExpectListWithMatchingServerNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.SEQNO, null);
        noticesParser.startElement(null, Notice.Fields.CATEGORY, null, null);
        noticesParser.characters("server".toCharArray(), 0, 6);
        noticesParser.endElement(null, Notice.Fields.CATEGORY, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.setSeqno(1);
        expected.setCategory("server");
        expected.setServerNotice(true);

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndCategoryScheduler_thenExpectListWithMatchingServerNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.SEQNO, null);
        noticesParser.startElement(null, Notice.Fields.CATEGORY, null, null);
        noticesParser.characters("scheduler".toCharArray(), 0, 9);
        noticesParser.endElement(null, Notice.Fields.CATEGORY, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.setSeqno(1);
        expected.setCategory("scheduler");
        expected.setServerNotice(true);

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndCategoryClient_thenExpectListWithMatchingClientNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.SEQNO, null);
        noticesParser.startElement(null, Notice.Fields.CATEGORY, null, null);
        noticesParser.characters("client".toCharArray(), 0, 6);
        noticesParser.endElement(null, Notice.Fields.CATEGORY, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.setSeqno(1);
        expected.setCategory("client");
        expected.setClientNotice(true);

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndLink_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.SEQNO, null);
        noticesParser.startElement(null, Notice.Fields.LINK, null, null);
        noticesParser.characters("Link".toCharArray(), 0, 4);
        noticesParser.endElement(null, Notice.Fields.LINK, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.setSeqno(1);
        expected.setLink("Link");

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }

    @Test
    public void testParser_whenXmlNoticeHasSeqnoAndProjectName_thenExpectListWithMatchingNotice()
            throws SAXException {
        noticesParser.startElement(null, NoticesParser.NOTICE_TAG, null, null);
        noticesParser.startElement(null, Notice.Fields.SEQNO, null, null);
        noticesParser.characters("1".toCharArray(), 0, 1);
        noticesParser.endElement(null, Notice.Fields.SEQNO, null);
        noticesParser.startElement(null, RPCCommonTags.PROJECT_NAME, null, null);
        noticesParser.characters("Project Name".toCharArray(), 0, "Project Name".length());
        noticesParser.endElement(null, RPCCommonTags.PROJECT_NAME, null);
        noticesParser.endElement(null, NoticesParser.NOTICE_TAG, null);

        expected.setSeqno(1);
        expected.setProjectName("Project Name");

        assertEquals(Collections.singletonList(expected), noticesParser.getNotices());
    }
}
