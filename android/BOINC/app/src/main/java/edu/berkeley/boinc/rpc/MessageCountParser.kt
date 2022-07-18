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

class MessageCountParser : BaseParser() {
    private var mParsed = false
    private var mInReply = false
    var seqno = -1

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(REPLY_TAG, ignoreCase = true)) {
            mInReply = true
        }
    }

    @Throws(SAXException::class)
    override fun characters(ch: CharArray, start: Int, length: Int) {
        super.characters(ch, start, length)
        mCurrentElement.setLength(0) // clear buffer after superclass operation
        // still empty - trim leading whitespace characters and append
        mCurrentElement.append(String(ch).trimStart())
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            trimEnd()
            if (localName.equals(REPLY_TAG, ignoreCase = true)) {
                mInReply = false
            } else if (mInReply && !mParsed && localName.equals("seqno", ignoreCase = true)) {
                seqno = mCurrentElement.toInt()
                mParsed = true
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "MessageCountParser.endElement error: ", e)
        }
        mCurrentElement.setLength(0)
    }

    companion object {
        const val REPLY_TAG = "boinc_gui_rpc_reply"
        @JvmStatic
        fun getSeqnoOfReply(reply: String?): Int {
            return try {
                val parser = MessageCountParser()
                Xml.parse(reply, parser)
                parser.seqno
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "MessageCountParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "MessageCountParser: $reply")

                -1
            }
        }
    }
}
