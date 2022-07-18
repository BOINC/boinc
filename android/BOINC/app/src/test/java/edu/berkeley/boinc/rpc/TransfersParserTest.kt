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
import edu.berkeley.boinc.rpc.TransfersParser.Companion.parse
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
class TransfersParserTest {
    private lateinit var transfersParser: TransfersParser
    private lateinit var expected: Transfer
    @Before
    fun setUp() {
        transfersParser = TransfersParser()
        expected = Transfer()
    }

    @Test
    fun `When Rpc string is null then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        Assert.assertTrue(parse(null).isEmpty())
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertTrue(parse("").isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When only start element is run then expect element started`() {
        transfersParser.startElement(null, "", null, null)
        Assert.assertTrue(transfersParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When both start element and end element are run then expect element not started`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        Assert.assertFalse(transfersParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect empty list`() {
        transfersParser.startElement(null, "", null, null)
        Assert.assertTrue(transfersParser.transfers.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml result has no elements then expect empty list`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        Assert.assertTrue(transfersParser.transfers.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name and project_url then expect list with matching Transfer`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.projectUrl = PROJECT_URL
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and generated_locally 0 then expect list with matching Transfer`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(
            null, Transfer.Fields.GENERATED_LOCALLY, null,
            null
        )
        transfersParser.characters("0".toCharArray(), 0, 1)
        transfersParser.endElement(null, Transfer.Fields.GENERATED_LOCALLY, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.projectUrl = PROJECT_URL
        expected.generatedLocally = false
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and generated_locally 1 then expect list with matching Transfer`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(
            null, Transfer.Fields.GENERATED_LOCALLY, null,
            null
        )
        transfersParser.characters("1".toCharArray(), 0, 1)
        transfersParser.endElement(null, Transfer.Fields.GENERATED_LOCALLY, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.projectUrl = PROJECT_URL
        expected.generatedLocally = true
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and is_upload 0 then expect list with matching Transfer`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(
            null, Transfer.Fields.IS_UPLOAD, null,
            null
        )
        transfersParser.characters("0".toCharArray(), 0, 1)
        transfersParser.endElement(null, Transfer.Fields.IS_UPLOAD, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.projectUrl = PROJECT_URL
        expected.isUpload = false
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and is_upload 1 then expect list with matching Transfer`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(
            null, Transfer.Fields.IS_UPLOAD, null,
            null
        )
        transfersParser.characters("1".toCharArray(), 0, 1)
        transfersParser.endElement(null, Transfer.Fields.IS_UPLOAD, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.projectUrl = PROJECT_URL
        expected.isUpload = true
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and invalid num_of_bytes then expect list with Transfer with only name and project_url`() {
        PowerMockito.mockStatic(Log::class.java)
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(null, Transfer.Fields.NBYTES, null, null)
        transfersParser.characters(
            "One thousand and five hundred".toCharArray(), 0,
            "One thousand and five hundred".length
        )
        transfersParser.endElement(null, Transfer.Fields.NBYTES, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.projectUrl = PROJECT_URL
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and num_of_bytes then expect list with matching Transfer`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(null, Transfer.Fields.NBYTES, null, null)
        transfersParser.characters("1500.5".toCharArray(), 0, 6)
        transfersParser.endElement(null, Transfer.Fields.NBYTES, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.projectUrl = PROJECT_URL
        expected.noOfBytes = 1500L
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and status then expect list with matching Transfer`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(null, Transfer.Fields.STATUS, null, null)
        transfersParser.characters("1".toCharArray(), 0, 1)
        transfersParser.endElement(null, Transfer.Fields.STATUS, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.projectUrl = PROJECT_URL
        expected.status = 1
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and time_so_far then expect list with matching Transfer`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(
            null, Transfer.Fields.TIME_SO_FAR, null,
            null
        )
        transfersParser.characters("1500.5".toCharArray(), 0, 6)
        transfersParser.endElement(null, Transfer.Fields.TIME_SO_FAR, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.projectUrl = PROJECT_URL
        expected.timeSoFar = 1500L
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and next_request_time hen expect list with matching Transfer`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(
            null, Transfer.Fields.NEXT_REQUEST_TIME, null,
            null
        )
        transfersParser.characters("1500.5".toCharArray(), 0, 6)
        transfersParser.endElement(null, Transfer.Fields.NEXT_REQUEST_TIME, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.projectUrl = PROJECT_URL
        expected.nextRequestTime = 1500L
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and last_bytes_transferred then expect list with Transfer with bytes_transferred equal to last_bytes_transferred`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(
            null, TransfersParser.LAST_BYTES_XFERRED_TAG, null,
            null
        )
        transfersParser.characters("1500.5".toCharArray(), 0, 6)
        transfersParser.endElement(null, TransfersParser.LAST_BYTES_XFERRED_TAG, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.projectUrl = PROJECT_URL
        expected.bytesTransferred = 1500L
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and project_backoff then expect list with matching Transfer`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(
            null, Transfer.Fields.PROJECT_BACKOFF, null,
            null
        )
        transfersParser.characters("1500.5".toCharArray(), 0, 6)
        transfersParser.endElement(null, Transfer.Fields.PROJECT_BACKOFF, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.projectUrl = PROJECT_URL
        expected.projectBackoff = 1500L
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and file_transfer_tag then expect list with matching Transfer`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(null, TransfersParser.FILE_XFER_TAG, null, null)
        transfersParser.endElement(null, TransfersParser.FILE_XFER_TAG, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.isTransferActive = true
        expected.projectUrl = PROJECT_URL
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url, last_bytes_transferred and file_transfer_with_bytes_transferred then expect list with Transfer with bytes_transferred not equal to last_bytes_transferred`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(
            null, Transfer.Fields.BYTES_XFERRED, null,
            null
        )
        transfersParser.startElement(
            null, TransfersParser.FILE_XFER_TAG, null,
            null
        )
        transfersParser.characters("2000.5".toCharArray(), 0, 6)
        transfersParser.endElement(null, Transfer.Fields.BYTES_XFERRED, null)
        transfersParser.endElement(null, TransfersParser.FILE_XFER_TAG, null)
        transfersParser.startElement(
            null, TransfersParser.LAST_BYTES_XFERRED_TAG, null,
            null
        )
        transfersParser.characters("1500.5".toCharArray(), 0, 6)
        transfersParser.endElement(null, TransfersParser.LAST_BYTES_XFERRED_TAG, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.isTransferActive = true
        expected.projectUrl = PROJECT_URL
        expected.bytesTransferred = 2000L
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Transfer has name, project_url and file_transfer with transfer_speed then expect list with matching Transfer`() {
        transfersParser.startElement(
            null, TransfersParser.FILE_TRANSFER_TAG, null,
            null
        )
        transfersParser.startElement(null, NAME, null, null)
        transfersParser.characters(TRANSFER_NAME.toCharArray(), 0, TRANSFER_NAME.length)
        transfersParser.endElement(null, NAME, null)
        transfersParser.startElement(
            null, edu.berkeley.boinc.rpc.PROJECT_URL, null,
            null
        )
        transfersParser.characters(PROJECT_URL.toCharArray(), 0, PROJECT_URL.length)
        transfersParser.endElement(null, edu.berkeley.boinc.rpc.PROJECT_URL, null)
        transfersParser.startElement(
            null, TransfersParser.FILE_XFER_TAG, null,
            null
        )
        transfersParser.startElement(
            null, Transfer.Fields.XFER_SPEED, null,
            null
        )
        transfersParser.characters("1000".toCharArray(), 0, 4)
        transfersParser.endElement(null, Transfer.Fields.XFER_SPEED, null)
        transfersParser.endElement(null, TransfersParser.FILE_XFER_TAG, null)
        transfersParser.endElement(null, TransfersParser.FILE_TRANSFER_TAG, null)
        expected.name = TRANSFER_NAME
        expected.isTransferActive = true
        expected.projectUrl = PROJECT_URL
        expected.transferSpeed = 1000f
        Assert.assertEquals(listOf(expected), transfersParser.transfers)
    }

    companion object {
        private const val TRANSFER_NAME = "Transfer"
        private const val PROJECT_URL = "Project URL"
    }
}
