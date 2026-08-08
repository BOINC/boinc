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

class AppsParser : BaseParser() {
    val apps: MutableList<App> = mutableListOf()
    private lateinit var mApp: App

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(APP_TAG, ignoreCase = true) && !this::mApp.isInitialized) {
            mApp = App()
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
            if (localName.equals(APP_TAG, ignoreCase = true)) { // Closing tag of <app> - add to vector and be ready for next one
                if (!mApp.name.isNullOrEmpty()) { // name is a must
                    apps.add(mApp)
                }
                mApp = App()
            } else { // Not the closing tag - we decode possible inner tags
                trimEnd()
                when {
                    localName.equals(NAME, ignoreCase = true) -> {
                        mApp.name = mCurrentElement.toString()
                    }
                    localName.equals(USER_FRIENDLY_NAME, ignoreCase = true) -> {
                        mApp.userFriendlyName = mCurrentElement.toString()
                    }
                    localName.equals(NON_CPU_INTENSIVE, ignoreCase = true) -> {
                        mApp.nonCpuIntensive = mCurrentElement.toInt()
                    }
                }
            }
            mElementStarted = false
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "AppsParser.endElement error: ", e)
        }
    }

    companion object {
        const val APP_TAG = "app"
        /**
         * Parse the RPC result (app) and generate vector of app
         *
         * @param rpcResult String returned by RPC call of core client
         * @return vector of app
         */
        @JvmStatic
        fun parse(rpcResult: String?): List<App> {
            return try {
                val parser = AppsParser()
                Xml.parse(rpcResult, parser)
                parser.apps
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "AppsParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "AppsParser: $rpcResult")

                emptyList()
            }
        }
    }
}
