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

import android.util.Xml
import edu.berkeley.boinc.utils.Logging
import org.xml.sax.Attributes
import org.xml.sax.SAXException

class TransfersParser : BaseParser() {
    val transfers: MutableList<Transfer> = mutableListOf()
    private lateinit var mTransfer: Transfer

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(FILE_TRANSFER_TAG, ignoreCase = true) && !this::mTransfer.isInitialized) {
            mTransfer = Transfer()
        } else if (localName.equals(FILE_XFER_TAG, ignoreCase = true)) {
            // Just constructor, flag should be set if it's present
            mTransfer.isTransferActive = true
        } else if (!localName.equals("persistent_file_xfer", ignoreCase = true)) {
            // Another element, hopefully primitive and not constructor
            // (although unknown constructor does not hurt, because there will be primitive start anyway)
            mElementStarted = true
            mCurrentElement.setLength(0)
        }
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            if (localName.equals(FILE_TRANSFER_TAG, ignoreCase = true)) {
                // Closing tag of <project> - add to list and be ready for next one
                if (mTransfer.projectUrl.isNotEmpty() && mTransfer.name.isNotEmpty()) { // project_url is a must
                    transfers.add(mTransfer)
                }
                mTransfer = Transfer()
            } else { // Not the closing tag - we decode possible inner tags
                trimEnd()
                if (localName.equals(PROJECT_URL, ignoreCase = true)) {
                    mTransfer.projectUrl = mCurrentElement.toString()
                } else if (localName.equals(NAME, ignoreCase = true)) {
                    mTransfer.name = mCurrentElement.toString()
                } else if (localName.equals(Transfer.Fields.GENERATED_LOCALLY, ignoreCase = true)) {
                    mTransfer.generatedLocally = mCurrentElement.toString() != "0"
                } else if (localName.equals(Transfer.Fields.IS_UPLOAD, ignoreCase = true)) {
                    mTransfer.isUpload = mCurrentElement.toString() != "0"
                } else if (localName.equals(Transfer.Fields.NBYTES, ignoreCase = true)) {
                    mTransfer.noOfBytes = mCurrentElement.toDouble().toLong()
                } else if (localName.equals(Transfer.Fields.STATUS, ignoreCase = true)) {
                    mTransfer.status = mCurrentElement.toInt()
                } else if (localName.equals(Transfer.Fields.TIME_SO_FAR, ignoreCase = true)) {
                    // inside <persistent_file_xfer>
                    mTransfer.timeSoFar = mCurrentElement.toDouble().toLong()
                } else if (localName.equals(Transfer.Fields.NEXT_REQUEST_TIME, ignoreCase = true)) {
                    // inside <persistent_file_xfer>
                    mTransfer.nextRequestTime = mCurrentElement.toDouble().toLong()
                } else if (localName.equals(LAST_BYTES_XFERRED_TAG, ignoreCase = true)) {
                    // inside <persistent_file_xfer>
                    // See also <bytes_xferred> below, both are setting the same parameters
                    if (mTransfer.bytesTransferred == 0L) { // Not set yet
                        mTransfer.bytesTransferred = mCurrentElement.toDouble().toLong()
                    }
                } else if (localName.equals(Transfer.Fields.BYTES_XFERRED, ignoreCase = true)) {
                    // Total bytes transferred, but this info is not available if networking
                    // is suspended. This info is present only inside <file_xfer> (active transfer)
                    // In such case we overwrite value set by <last_bytes_xferred>
                    mTransfer.bytesTransferred = mCurrentElement.toDouble().toLong()
                } else if (localName.equals(Transfer.Fields.XFER_SPEED, ignoreCase = true)) { // inside <file_xfer>
                    mTransfer.transferSpeed = mCurrentElement.toFloat()
                } else if (localName.equals(Transfer.Fields.PROJECT_BACKOFF, ignoreCase = true)) {
                    mTransfer.projectBackoff = mCurrentElement.toDouble().toLong()
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "TransfersParser.endElement error: ", e)
        }
        mElementStarted = false
    }

    companion object {
        const val FILE_TRANSFER_TAG = "file_transfer"
        const val FILE_XFER_TAG = "file_xfer"
        const val LAST_BYTES_XFERRED_TAG = "last_bytes_xferred"
        /**
         * Parse the RPC result (projects) and generate vector of projects info
         *
         * @param rpcResult String returned by RPC call of core client
         * @return vector of projects info
         */
        @JvmStatic
        fun parse(rpcResult: String?): List<Transfer> {
            return try {
                val parser = TransfersParser()
                Xml.parse(rpcResult, parser)
                parser.transfers
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "TransfersParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "TransfersParser: $rpcResult")

                emptyList()
            }
        }
    }
}
