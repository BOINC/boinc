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

class CcStatusParser : BaseParser() {
    lateinit var ccStatus: CcStatus
        private set

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(CC_STATUS_TAG, ignoreCase = true)) {
            ccStatus = CcStatus()
        } else { // Another element, hopefully primitive and not constructor
            // (although unknown constructor does not hurt, because there will be primitive start anyway)
            mElementStarted = true
            mCurrentElement.setLength(0)
        }
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            // Closing tag of <cc_status> - nothing to do at the moment
            if (!localName.equals(CC_STATUS_TAG, ignoreCase = true)) {
                trimEnd()
                // Not the closing tag - we decode possible inner tags
                when {
                    localName.equals(CcStatus.Fields.TASK_MODE, ignoreCase = true) -> {
                        ccStatus.taskMode = mCurrentElement.toInt()
                    }
                    localName.equals(CcStatus.Fields.TASK_MODE_PERM, ignoreCase = true) -> {
                        ccStatus.taskModePerm = mCurrentElement.toInt()
                    }
                    localName.equals(CcStatus.Fields.TASK_MODE_DELAY, ignoreCase = true) -> {
                        ccStatus.taskModeDelay = mCurrentElement.toDouble()
                    }
                    localName.equals(CcStatus.Fields.TASK_SUSPEND_REASON, ignoreCase = true) -> {
                        ccStatus.taskSuspendReason = mCurrentElement.toInt()
                    }
                }
                if (localName.equals(CcStatus.Fields.NETWORK_MODE, ignoreCase = true)) {
                    ccStatus.networkMode = mCurrentElement.toInt()
                } else if (localName.equals(CcStatus.Fields.NETWORK_MODE_PERM, ignoreCase = true)) {
                    ccStatus.networkModePerm = mCurrentElement.toInt()
                } else if (localName.equals(CcStatus.Fields.NETWORK_MODE_DELAY, ignoreCase = true)) {
                    ccStatus.networkModeDelay = mCurrentElement.toDouble()
                } else if (localName.equals(CcStatus.Fields.NETWORK_SUSPEND_REASON, ignoreCase = true)) {
                    ccStatus.networkSuspendReason = mCurrentElement.toInt()
                } else if (localName.equals(CcStatus.Fields.NETWORK_STATUS, ignoreCase = true)) {
                    ccStatus.networkStatus = mCurrentElement.toInt()
                } else if (localName.equals(CcStatus.Fields.AMS_PASSWORD_ERROR, ignoreCase = true)) {
                    if (mCurrentElement.length > 1) {
                        ccStatus.amsPasswordError = 0 != mCurrentElement.toInt()
                    } else {
                        ccStatus.amsPasswordError = true
                    }
                } else if (localName.equals(CcStatus.Fields.MANAGER_MUST_QUIT, ignoreCase = true)) {
                    if (mCurrentElement.length > 1) {
                        ccStatus.managerMustQuit = 0 != mCurrentElement.toInt()
                    } else {
                        ccStatus.managerMustQuit = true
                    }
                } else if (localName.equals(CcStatus.Fields.DISALLOW_ATTACH, ignoreCase = true)) {
                    if (mCurrentElement.length > 1) {
                        ccStatus.disallowAttach = 0 != mCurrentElement.toInt()
                    } else {
                        ccStatus.disallowAttach = true
                    }
                } else if (localName.equals(CcStatus.Fields.SIMPLE_GUI_ONLY, ignoreCase = true)) {
                    if (mCurrentElement.length > 1) {
                        ccStatus.simpleGuiOnly = 0 != mCurrentElement.toInt()
                    } else {
                        ccStatus.simpleGuiOnly = true
                    }
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "CcStatusParser.endElement error: ", e)
        }
        mElementStarted = false
    }

    companion object {
        const val CC_STATUS_TAG = "cc_status"
        @JvmStatic
        fun parse(rpcResult: String?): CcStatus? {
            return try {
                val parser = CcStatusParser()
                Xml.parse(rpcResult, parser)
                parser.ccStatus
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "CcStatusParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "CcStatusParser: $rpcResult")

                null
            }
        }
    }
}
