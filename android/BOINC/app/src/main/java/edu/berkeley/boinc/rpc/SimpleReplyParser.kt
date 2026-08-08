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

class SimpleReplyParser : BaseParser() {
    private var mParsed = false
    private var mInReply = false
    var result = false
        private set
    var errorMessage: String? = null
        private set

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(MessageCountParser.REPLY_TAG, ignoreCase = true)) {
            mInReply = true
        } else {
            mElementStarted = true
        }
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            if (localName.equals(MessageCountParser.REPLY_TAG, ignoreCase = true)) {
                mInReply = false
            } else if (mInReply && !mParsed) {
                when {
                    localName.equals("success", ignoreCase = true) -> {
                        result = true
                        mParsed = true
                    }
                    localName.equals("failure", ignoreCase = true) -> {
                        result = false
                        mParsed = true
                    }
                    localName.equals("error", ignoreCase = true) -> {
                        trimEnd()
                        errorMessage = mCurrentElement.toString()
                        result = false
                        mParsed = true
                    }
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "SimpleReplyParser.endElement error: ", e)
        }
        mElementStarted = false
    }

    companion object {
        @JvmStatic
        fun parse(reply: String?): SimpleReplyParser? {
            return try {
                val parser = SimpleReplyParser()
                Xml.parse(reply, parser)
                parser
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "SimpleReplyParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "SimpleReplyParser: $reply")

                null
            }
        }
    }
}
