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

class AcctMgrInfoParser : BaseParser() {
    lateinit var accountMgrInfo: AcctMgrInfo
        private set

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(ACCT_MGR_INFO_TAG, ignoreCase = true)) {
            accountMgrInfo = AcctMgrInfo()
        } else {
            mElementStarted = true
            mCurrentElement.setLength(0)
        }
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            if (localName.equals(ACCT_MGR_INFO_TAG, ignoreCase = true)) { // closing tag
                if (arrayOf(accountMgrInfo.acctMgrName, accountMgrInfo.acctMgrUrl)
                                .none { it.isEmpty() } &&
                        accountMgrInfo.isHavingCredentials) {
                    accountMgrInfo.isPresent = true
                }
            } else { // decode inner tags
                when {
                    localName.equals(AcctMgrInfo.Fields.ACCT_MGR_NAME, ignoreCase = true) -> {
                        accountMgrInfo.acctMgrName = mCurrentElement.toString()
                    }
                    localName.equals(AcctMgrInfo.Fields.ACCT_MGR_URL, ignoreCase = true) -> {
                        accountMgrInfo.acctMgrUrl = mCurrentElement.toString()
                    }
                    localName.equals(AcctMgrInfo.Fields.HAVING_CREDENTIALS, ignoreCase = true) -> {
                        accountMgrInfo.isHavingCredentials = true
                    }
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "AcctMgrInfoParser.endElement error: ", e)
        }
        mElementStarted = false
    }

    companion object {
        const val ACCT_MGR_INFO_TAG = "acct_mgr_info"
        @JvmStatic
        fun parse(rpcResult: String?): AcctMgrInfo? {
            return try {
                val parser = AcctMgrInfoParser()
                Xml.parse(rpcResult, parser)
                parser.accountMgrInfo
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "AcctMgrInfoParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "AcctMgrInfoParser: $rpcResult")

                null
            }
        }
    }
}
