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

class AcctMgrRPCReplyParser : BaseParser() {
    lateinit var accountMgrRPCReply: AcctMgrRPCReply
        private set

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(ACCT_MGR_RPC_REPLY_TAG, ignoreCase = true)) {
            accountMgrRPCReply = AcctMgrRPCReply()
        } else {
            mElementStarted = true
            mCurrentElement.setLength(0)
        }
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            if (!localName.equals(ACCT_MGR_RPC_REPLY_TAG, ignoreCase = true)) {
                // not closing tag, decode inner tags
                if (localName.equals(ERROR_NUM, ignoreCase = true)) {
                    accountMgrRPCReply.errorNum = mCurrentElement.toInt()
                } else if (localName.equals(MESSAGE, ignoreCase = true)) {
                    accountMgrRPCReply.messages.add(mCurrentElement.toString())
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "AcctMgrRPCReplyParser.endElement error: ", e)
        }
        mElementStarted = false
    }

    companion object {
        const val ACCT_MGR_RPC_REPLY_TAG = "acct_mgr_rpc_reply"

        @JvmStatic
        fun parse(rpcResult: String): AcctMgrRPCReply? {
            return try {
                val parser = AcctMgrRPCReplyParser()
                Xml.parse(rpcResult.replace("<success/>", "<success>1</success>"), parser)
                parser.accountMgrRPCReply
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "AcctMgrRPCReplyParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "AcctMgrRPCReplyParser: $rpcResult")

                null
            }
        }
    }
}
