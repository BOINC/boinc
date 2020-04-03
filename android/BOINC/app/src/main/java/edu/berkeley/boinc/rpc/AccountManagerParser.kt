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

class AccountManagerParser : BaseParser() {
    val accountManagerInfos: MutableList<AccountManager> = mutableListOf()
    private lateinit var mAcctMgrInfo: AccountManager

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(ACCOUNT_MANAGER, ignoreCase = true) && !this::mAcctMgrInfo.isInitialized) {
            mAcctMgrInfo = AccountManager()
        } else {
            mElementStarted = true
            mCurrentElement.setLength(0)
        }
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            if (localName.equals(ACCOUNT_MANAGER, ignoreCase = true)) {
                if (mAcctMgrInfo.name.isNotEmpty()) { // name is a must
                    accountManagerInfos.add(mAcctMgrInfo)
                }
                mAcctMgrInfo = AccountManager()
            } else { // Not the closing tag - we decode possible inner tags
                trimEnd()
                when {
                    localName.equals(NAME, ignoreCase = true) -> { //project name
                        mAcctMgrInfo.name = mCurrentElement.toString()
                    }
                    localName.equals(URL, ignoreCase = true) -> {
                        mAcctMgrInfo.url = mCurrentElement.toString()
                    }
                    localName.equals(DESCRIPTION, ignoreCase = true) -> {
                        mAcctMgrInfo.description = mCurrentElement.toString()
                    }
                    localName.equals(IMAGE_TAG, ignoreCase = true) -> {
                        mAcctMgrInfo.imageUrl = mCurrentElement.toString()
                    }
                }
            }
        } catch (e: Exception) {
            if (Logging.ERROR) Log.e(Logging.TAG, "AccountManagerParser.endElement error: ", e)
        }
        mElementStarted = false
    }

    companion object {
        const val IMAGE_TAG = "image"
        @JvmStatic
		fun parse(rpcResult: String): List<AccountManager> {
            return try {
                val parser = AccountManagerParser()
                Xml.parse(rpcResult, parser)
                parser.accountManagerInfos
            } catch (e: SAXException) {
                emptyList()
            }
        }
    }
}
