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

class WorkUnitsParser : BaseParser() {
    val workUnits: MutableList<WorkUnit> = mutableListOf()
    private var mWorkUnit: WorkUnit? = null

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(WORKUNIT_TAG, ignoreCase = true)) {
            mWorkUnit = WorkUnit()
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
            if (mWorkUnit != null) { // We are inside <workunit>
                if (localName.equals(WORKUNIT_TAG, ignoreCase = true)) { // Closing tag of <workunit> - add to vector and be ready for next one
                    if (mWorkUnit!!.name.isNotEmpty()) { // name is a must
                        this.workUnits.add(mWorkUnit!!)
                    }
                    mWorkUnit = null
                } else { // Not the closing tag - we decode possible inner tags
                    trimEnd()
                    when {
                        localName.equals(NAME, ignoreCase = true) -> {
                            mWorkUnit!!.name = mCurrentElement.toString()
                        }
                        localName.equals(WorkUnit.Fields.APP_NAME, ignoreCase = true) -> {
                            mWorkUnit!!.appName = mCurrentElement.toString()
                        }
                        localName.equals(WorkUnit.Fields.VERSION_NUM, ignoreCase = true) -> {
                            mWorkUnit!!.versionNum = mCurrentElement.toInt()
                        }
                        localName.equals(WorkUnit.Fields.RSC_FPOPS_EST, ignoreCase = true) -> {
                            mWorkUnit!!.rscFloatingPointOpsEst = mCurrentElement.toDouble()
                        }
                        localName.equals(WorkUnit.Fields.RSC_FPOPS_BOUND, ignoreCase = true) -> {
                            mWorkUnit!!.rscFloatingPointOpsBound = mCurrentElement.toDouble()
                        }
                        localName.equals(WorkUnit.Fields.RSC_MEMORY_BOUND, ignoreCase = true) -> {
                            mWorkUnit!!.rscMemoryBound = mCurrentElement.toDouble()
                        }
                        localName.equals(WorkUnit.Fields.RSC_DISK_BOUND, ignoreCase = true) -> {
                            mWorkUnit!!.rscDiskBound = mCurrentElement.toDouble()
                        }
                    }
                }
            }
        } catch (e: NumberFormatException) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "WorkunitsParser.endElement error: ", e)
            }
        }
        mElementStarted = false
    }

    companion object {
        const val WORKUNIT_TAG = "workunit"
        /**
         * Parse the RPC result (workunit) and generate corresponding vector
         *
         * @param rpcResult String returned by RPC call of core client
         * @return vector of workunits
         */
        @JvmStatic
        fun parse(rpcResult: String?): List<WorkUnit> {
            return try {
                val parser = WorkUnitsParser()
                Xml.parse(rpcResult, parser)
                parser.workUnits
            } catch (e: SAXException) {
                emptyList()
            }
        }
    }
}