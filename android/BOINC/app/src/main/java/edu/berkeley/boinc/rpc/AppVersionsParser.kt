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
import edu.berkeley.boinc.utils.Logging
import org.xml.sax.Attributes
import org.xml.sax.SAXException

class AppVersionsParser : BaseParser() {
    val appVersions: MutableList<AppVersion> = mutableListOf()
    private var mAppVersion: AppVersion? = null

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(APP_VERSION_TAG, ignoreCase = true) && mAppVersion == null) {
            mAppVersion = AppVersion()
        }
    }

    @Throws(SAXException::class)
    override fun characters(ch: CharArray, start: Int, length: Int) {
        super.characters(ch, start, length)
        // put it into StringBuilder
        var myStart = start
        var myLength = length
        if (mCurrentElement.isEmpty()) {
            // still empty - trim leading white-spaces
            while (myStart < length) {
                if (!Character.isWhitespace(ch[myStart])) {
                    // First non-white-space character
                    break
                }
                ++myStart
                --myLength
            }
        }
        mCurrentElement.append(ch, myStart, myLength)
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            trimEnd()
            if (localName.equals(APP_VERSION_TAG, ignoreCase = true)) { // Closing tag of <app_version> - add to vector and be ready for next one
                if (!mAppVersion?.appName.isNullOrEmpty()) { // appName is a must
                    appVersions.add(mAppVersion!!)
                }
                mAppVersion = null
            } else { // Not the closing tag - we decode possible inner tags
                if (localName.equals(AppVersion.Fields.APP_NAME, ignoreCase = true)) {
                    mAppVersion?.appName = mCurrentElement.toString()
                } else if (localName.equals(AppVersion.Fields.VERSION_NUM, ignoreCase = true)) {
                    mAppVersion?.versionNum = mCurrentElement.toInt()
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "AppVersionsParser.endElement error: ", e)
        }
        mCurrentElement.setLength(0) // to be clean for next one
    }

    companion object {
        const val APP_VERSION_TAG = "app_version"
        /**
         * Parse the RPC result (app_version) and generate corresponding vector
         *
         * @param rpcResult String returned by RPC call of core client
         * @return vector of application version
         */
        @JvmStatic
        fun parse(rpcResult: String?): List<AppVersion> {
            return try {
                val parser = AppVersionsParser()
                Xml.parse(rpcResult, parser)
                parser.appVersions
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "AppVersionsParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "AppVersionsParser: $rpcResult")

                emptyList()
            }
        }
    }
}
