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

class ProjectAttachReplyParser : BaseParser() {
    lateinit var projectAttachReply: ProjectAttachReply
        private set

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(PROJECT_ATTACH_REPLY_TAG, ignoreCase = true)) {
            projectAttachReply = ProjectAttachReply()
        } else {
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
            if (!localName.equals(PROJECT_ATTACH_REPLY_TAG, ignoreCase = true)) {
                // Not the closing tag - we decode possible inner tags
                trimEnd()
                if (localName.equals(ERROR_NUM_TAG, ignoreCase = true)) {
                    projectAttachReply.errorNum = mCurrentElement.toInt()
                } else if (localName.equals(MESSAGE, ignoreCase = true)) {
                    projectAttachReply.messages.add(mCurrentElement.toString())
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "ProjectAttachReplyParser.endElement error: ", e)
        }
        mElementStarted = false
    }

    companion object {
        const val PROJECT_ATTACH_REPLY_TAG = "project_attach_reply"
        const val ERROR_NUM_TAG = "error_num"

        @JvmStatic
        fun parse(rpcResult: String?): ProjectAttachReply? {
            return try {
                val parser = ProjectAttachReplyParser()
                Xml.parse(rpcResult, parser)
                parser.projectAttachReply
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "ProjectAttachReplyParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "ProjectAttachReplyParser: $rpcResult")

                null
            }
        }
    }
}
