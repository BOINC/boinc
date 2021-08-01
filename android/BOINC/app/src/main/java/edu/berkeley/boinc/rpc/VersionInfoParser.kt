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

class VersionInfoParser : BaseParser() {
    lateinit var versionInfo: VersionInfo
        private set

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(SERVER_VERSION_TAG, ignoreCase = true)) {
            versionInfo = VersionInfo()
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
            if (!localName.equals(SERVER_VERSION_TAG, ignoreCase = true)) { // Not the closing tag - we decode possible inner tags
                trimEnd()
                when {
                    localName.equals(VersionInfo.Fields.MAJOR, ignoreCase = true) -> {
                        versionInfo.major = mCurrentElement.toInt()
                    }
                    localName.equals(VersionInfo.Fields.MINOR, ignoreCase = true) -> {
                        versionInfo.minor = mCurrentElement.toInt()
                    }
                    localName.equals(VersionInfo.Fields.RELEASE, ignoreCase = true) -> {
                        versionInfo.release = mCurrentElement.toInt()
                    }
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "VersionInfoParser.endElement error: ", e)
        }
        mElementStarted = false // to be clean for next one
    }

    companion object {
        const val SERVER_VERSION_TAG = "server_version"
        /**
         * Parse the RPC result (host_info) and generate vector of projects info
         *
         * @param rpcResult String returned by RPC call of core client
         * @return VersionInfo (of core client)
         */
        @JvmStatic
        fun parse(rpcResult: String?): VersionInfo? {
            return try {
                val parser = VersionInfoParser()
                Xml.parse(rpcResult, parser)
                parser.versionInfo
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "VersionInfoParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "VersionInfoParser: $rpcResult")

                null
            }
        }
    }
}
