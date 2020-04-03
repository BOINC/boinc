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
package edu.berkeley.boinc.rpc

import android.util.Log
import android.util.Xml
import edu.berkeley.boinc.utils.Logging
import org.xml.sax.Attributes
import org.xml.sax.SAXException

class AccountOutParser : BaseParser() {
    var accountOut: AccountOut? = null
        private set

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equalsAny(ERROR_NUM, AccountOut.Fields.ERROR_MSG,
                        AccountOut.Fields.AUTHENTICATOR, ignoreCase = true)) {
            if (accountOut == null) {
                accountOut = AccountOut()
            }
        } else {
            mElementStarted = true
            mCurrentElement.setLength(0)
        }
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            if (accountOut != null) {
                trimEnd()
                when {
                    localName.equals(ERROR_NUM, ignoreCase = true) -> {
                        accountOut!!.errorNum = mCurrentElement.toInt()
                    }
                    localName.equals(AccountOut.Fields.ERROR_MSG, ignoreCase = true) -> {
                        accountOut!!.errorMsg = mCurrentElement.toString()
                    }
                    localName.equals(AccountOut.Fields.AUTHENTICATOR, ignoreCase = true) -> {
                        accountOut!!.authenticator = mCurrentElement.toString()
                    }
                }
            }
        } catch (e: NumberFormatException) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "AccountOutParser.endElement error: ", e)
            }
        }
        mElementStarted = false
    }

    companion object {
        @JvmStatic
        fun parse(rpcResult: String): AccountOut? {
            return try {
                var outResult: String?
                val xmlHeaderStart = rpcResult.indexOf("<?xml")
                if (xmlHeaderStart != -1) {
                    val xmlHeaderEnd = rpcResult.indexOf("?>")
                    outResult = rpcResult.substring(0, xmlHeaderStart)
                    outResult += rpcResult.substring(xmlHeaderEnd + 2)
                } else {
                    outResult = rpcResult
                }
                val parser = AccountOutParser()
                Xml.parse(outResult, parser)
                parser.accountOut
            } catch (e: SAXException) {
                null
            }
        }
    }
}
