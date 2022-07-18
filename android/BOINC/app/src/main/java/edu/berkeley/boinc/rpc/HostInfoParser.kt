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

class HostInfoParser : BaseParser() {
    lateinit var hostInfo: HostInfo
        private set

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(HOST_INFO_TAG, ignoreCase = true)) {
            hostInfo = HostInfo()
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
            if (!localName.equals(HOST_INFO_TAG, ignoreCase = true)) { // Not the closing tag - we decode possible inner tags
                trimEnd()
                when {
                    localName.equals(HostInfo.Fields.TIMEZONE, ignoreCase = true) -> {
                        hostInfo.timezone = mCurrentElement.toInt()
                    }
                    localName.equals(HostInfo.Fields.DOMAIN_NAME, ignoreCase = true) -> {
                        hostInfo.domainName = mCurrentElement.toString()
                    }
                    localName.equals(HostInfo.Fields.IP_ADDR, ignoreCase = true) -> {
                        hostInfo.ipAddress = mCurrentElement.toString()
                    }
                    localName.equals(HostInfo.Fields.HOST_CPID, ignoreCase = true) -> {
                        hostInfo.hostCpid = mCurrentElement.toString()
                    }
                    localName.equals(HostInfo.Fields.P_NCPUS, ignoreCase = true) -> {
                        hostInfo.noOfCPUs = mCurrentElement.toInt()
                    }
                    localName.equals(HostInfo.Fields.P_VENDOR, ignoreCase = true) -> {
                        hostInfo.cpuVendor = mCurrentElement.toString()
                    }
                    localName.equals(HostInfo.Fields.P_MODEL, ignoreCase = true) -> {
                        hostInfo.cpuModel = mCurrentElement.toString()
                    }
                    localName.equals(HostInfo.Fields.P_FEATURES, ignoreCase = true) -> {
                        hostInfo.cpuFeatures = mCurrentElement.toString()
                    }
                    localName.equals(HostInfo.Fields.P_FPOPS, ignoreCase = true) -> {
                        hostInfo.cpuFloatingPointOps = mCurrentElement.toDouble()
                    }
                    localName.equals(HostInfo.Fields.P_IOPS, ignoreCase = true) -> {
                        hostInfo.cpuIntegerOps = mCurrentElement.toDouble()
                    }
                    localName.equals(HostInfo.Fields.P_MEMBW, ignoreCase = true) -> {
                        hostInfo.cpuMembw = mCurrentElement.toDouble()
                    }
                    localName.equals(HostInfo.Fields.P_CALCULATED, ignoreCase = true) -> {
                        hostInfo.cpuCalculated = mCurrentElement.toDouble().toLong()
                    }
                    localName.equals(HostInfo.Fields.PRODUCT_NAME, ignoreCase = true) -> {
                        hostInfo.productName = mCurrentElement.toString()
                    }
                    localName.equals(HostInfo.Fields.M_NBYTES, ignoreCase = true) -> {
                        hostInfo.memoryInBytes = mCurrentElement.toDouble()
                    }
                    localName.equals(HostInfo.Fields.M_CACHE, ignoreCase = true) -> {
                        hostInfo.memoryCache = mCurrentElement.toDouble()
                    }
                    localName.equals(HostInfo.Fields.M_SWAP, ignoreCase = true) -> {
                        hostInfo.memorySwap = mCurrentElement.toDouble()
                    }
                    localName.equals(HostInfo.Fields.D_TOTAL, ignoreCase = true) -> {
                        hostInfo.totalDiskSpace = mCurrentElement.toDouble()
                    }
                    localName.equals(HostInfo.Fields.D_FREE, ignoreCase = true) -> {
                        hostInfo.freeDiskSpace = mCurrentElement.toDouble()
                    }
                    localName.equals(HostInfo.Fields.OS_NAME, ignoreCase = true) -> {
                        hostInfo.osName = mCurrentElement.toString()
                    }
                    localName.equals(HostInfo.Fields.OS_VERSION, ignoreCase = true) -> {
                        hostInfo.osVersion = mCurrentElement.toString()
                    }
                    localName.equals(HostInfo.Fields.VIRTUALBOX_VERSION, ignoreCase = true) -> {
                        hostInfo.virtualBoxVersion = mCurrentElement.toString()
                    }
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "HostInfoParser.endElement error: ", e)
        }
        mElementStarted = false
    }

    companion object {
        const val HOST_INFO_TAG = "host_info"
        /**
         * Parse the RPC result (host_info) and generate vector of projects info
         *
         * @param rpcResult String returned by RPC call of core client
         * @return HostInfo
         */
        @JvmStatic
        fun parse(rpcResult: String?): HostInfo? {
            return try {
                val parser = HostInfoParser()
                Xml.parse(rpcResult, parser)
                parser.hostInfo
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "HostInfoParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "HostInfoParser: $rpcResult")

                null
            }
        }
    }
}
