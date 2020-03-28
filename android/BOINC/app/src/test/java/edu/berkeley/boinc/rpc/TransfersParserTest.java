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

import java.util.Collections;

import static org.junit.Assert.*;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class TransfersParserTest {
    private static final String TRANSFER_NAME = "Transfer";
    private static final String PROJECT_URL = "Project URL";

    private TransfersParser transfersParser;
    private Transfer expected;

    @Before
    public void setUp() {
        transfersParser = new TransfersParser();
        expected = new Transfer();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectEmptyList() {
        mockStatic(Xml.class);

        assertEquals(Collections.emptyList(), TransfersParser.parse(null));
    }

    @Test
    public void testParser_whenOnlyStartElementIsRun_thenExpectElementStarted() throws SAXException {
        transfersParser.startElement(null, "", null, null);

        assertTrue(transfersParser.mElementStarted);
    }

    @Test
    public void testParser_whenBothStartElementAndEndElementAreRun_thenExpectElementNotStarted()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        assertFalse(transfersParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectEmptyList() throws SAXException {
        transfersParser.startElement(null, "", null, null);

        assertTrue(transfersParser.getTransfers().isEmpty());
    }

    @Test
    public void testParser_whenXmlResultHasNoElements_thenExpectEmptyList()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        assertTrue(transfersParser.getTransfers().isEmpty());
    }

    @Test
    public void testParser_whenXmlTransferHasNameAndProjectUrl_thenExpectListWithMatchingTransfer()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.project_url = PROJECT_URL;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndGeneratedLocally0_thenExpectListWithMatchingTransfer()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, Transfer.Fields.generated_locally, null,
                                     null);
        transfersParser.characters("0".toCharArray(), 0, 1);
        transfersParser.endElement(null, Transfer.Fields.generated_locally, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.project_url = PROJECT_URL;
        expected.generated_locally = false;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndGeneratedLocally1_thenExpectListWithMatchingTransfer()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, Transfer.Fields.generated_locally, null,
                                     null);
        transfersParser.characters("1".toCharArray(), 0, 1);
        transfersParser.endElement(null, Transfer.Fields.generated_locally, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.project_url = PROJECT_URL;
        expected.generated_locally = true;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndIsUpload0_thenExpectListWithMatchingTransfer()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, Transfer.Fields.is_upload, null,
                                     null);
        transfersParser.characters("0".toCharArray(), 0, 1);
        transfersParser.endElement(null, Transfer.Fields.is_upload, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.project_url = PROJECT_URL;
        expected.is_upload = false;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndIsUpload1_thenExpectListWithMatchingTransfer()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, Transfer.Fields.is_upload, null,
                                     null);
        transfersParser.characters("1".toCharArray(), 0, 1);
        transfersParser.endElement(null, Transfer.Fields.is_upload, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.project_url = PROJECT_URL;
        expected.is_upload = true;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndInvalidNumOfBytes_thenExpectListWithTransferWithOnlyNameAndProjectUrl()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, Transfer.Fields.nbytes, null, null);
        transfersParser.characters("One thousand and five hundred".toCharArray(), 0,
                                   "One thousand and five hundred".length());
        transfersParser.endElement(null, Transfer.Fields.nbytes, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.project_url = PROJECT_URL;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndNumOfBytes_thenExpectListWithMatchingTransfer()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, Transfer.Fields.nbytes, null, null);
        transfersParser.characters("1500.5".toCharArray(), 0, 6);
        transfersParser.endElement(null, Transfer.Fields.nbytes, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.project_url = PROJECT_URL;
        expected.nbytes = 1500L;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndStatus_thenExpectListWithMatchingTransfer()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, Transfer.Fields.status, null, null);
        transfersParser.characters("1".toCharArray(), 0, 1);
        transfersParser.endElement(null, Transfer.Fields.status, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.project_url = PROJECT_URL;
        expected.status = 1;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndTimeSoFar_thenExpectListWithMatchingTransfer()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, Transfer.Fields.time_so_far, null,
                                     null);
        transfersParser.characters("1500.5".toCharArray(), 0, 6);
        transfersParser.endElement(null, Transfer.Fields.time_so_far, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.project_url = PROJECT_URL;
        expected.time_so_far = 1500L;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndNextRequestTime_thenExpectListWithMatchingTransfer()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, Transfer.Fields.next_request_time, null,
                                     null);
        transfersParser.characters("1500.5".toCharArray(), 0, 6);
        transfersParser.endElement(null, Transfer.Fields.next_request_time, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.project_url = PROJECT_URL;
        expected.next_request_time = 1500L;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndLastBytesTransferred_thenExpectListWithTransferWithBytesTransferredEqualToLastBytesTransferred()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, TransfersParser.LAST_BYTES_XFERRED_TAG, null,
                                     null);
        transfersParser.characters("1500.5".toCharArray(), 0, 6);
        transfersParser.endElement(null, TransfersParser.LAST_BYTES_XFERRED_TAG, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.project_url = PROJECT_URL;
        expected.bytes_xferred = 1500L;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndProjectBackoff_thenExpectListWithMatchingTransfer()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, Transfer.Fields.project_backoff, null,
                                     null);
        transfersParser.characters("1500.5".toCharArray(), 0, 6);
        transfersParser.endElement(null, Transfer.Fields.project_backoff, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.project_url = PROJECT_URL;
        expected.project_backoff = 1500L;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndFileTransferTag_thenExpectListWithMatchingTransfer()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, TransfersParser.FILE_XFER_TAG, null, null);
        transfersParser.endElement(null, TransfersParser.FILE_XFER_TAG, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.xfer_active = true;
        expected.project_url = PROJECT_URL;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlLastBytesTransferredAndFileTransferWithBytesTransferred_thenExpectListWithTransferWithBytesTransferredNotEqualToLastBytesTransferred()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, Transfer.Fields.bytes_xferred, null,
                                     null);
        transfersParser.startElement(null, TransfersParser.FILE_XFER_TAG, null,
                                     null);
        transfersParser.characters("2000.5".toCharArray(), 0, 6);
        transfersParser.endElement(null, Transfer.Fields.bytes_xferred, null);
        transfersParser.endElement(null, TransfersParser.FILE_XFER_TAG, null);
        transfersParser.startElement(null, TransfersParser.LAST_BYTES_XFERRED_TAG, null,
                                     null);
        transfersParser.characters("1500.5".toCharArray(), 0, 6);
        transfersParser.endElement(null, TransfersParser.LAST_BYTES_XFERRED_TAG, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.xfer_active = true;
        expected.project_url = PROJECT_URL;
        expected.bytes_xferred = 2000L;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }

    @Test
    public void testParser_whenXmlTransferHasNameProjectUrlAndFileTransferWithTransferSpeed_thenExpectListWithMatchingTransfer()
            throws SAXException {
        transfersParser.startElement(null, TransfersParser.FILE_TRANSFER_TAG, null,
                                     null);
        transfersParser.startElement(null, RPCCommonTags.NAME, null, null);
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length());
        transfersParser.endElement(null, RPCCommonTags.NAME, null);
        transfersParser.startElement(null, RPCCommonTags.PROJECT_URL, null,
                                     null);
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length());
        transfersParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        transfersParser.startElement(null, TransfersParser.FILE_XFER_TAG, null,
                                     null);
        transfersParser.startElement(null, Transfer.Fields.xfer_speed, null,
                                     null);
        transfersParser.characters("1000".toCharArray(), 0, 4);
        transfersParser.endElement(null, Transfer.Fields.xfer_speed, null);
        transfersParser.endElement(null, TransfersParser.FILE_XFER_TAG, null);
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null);

        expected.name = TRANSFER_NAME;
        expected.xfer_active = true;
        expected.project_url = PROJECT_URL;
        expected.xfer_speed = 1000L;

        assertEquals(Collections.singletonList(expected), transfersParser.getTransfers());
    }
}
