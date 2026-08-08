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

class ResultsParser : BaseParser() {
    val results: MutableList<Result> = mutableListOf()
    private lateinit var mResult: Result
    private var mInActiveTask = false

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        when {
            localName.equals(RESULT_TAG, ignoreCase = true) && !this::mResult.isInitialized -> {
                mResult = Result()
            }
            localName.equals(Result.Fields.ACTIVE_TASK, ignoreCase = true) -> {
                mInActiveTask = true
            }
            else -> {
                // Another element, hopefully primitive and not constructor
                // (although unknown constructor does not hurt, because there will be primitive start anyway)
                mElementStarted = true
                mCurrentElement.setLength(0)
            }
        }
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            if (localName.equals(RESULT_TAG, ignoreCase = true)) {
                // Closing tag of <result> - add to list and be ready for next one
                if (mResult.name.isNotEmpty()) { // name is a must
                    results.add(mResult)
                }
                mResult = Result()
            } else { // Not the closing tag - we decode possible inner tags
                trimEnd()
                if (mInActiveTask) { // we are in <active_task>
                    when {
                        localName.equals(Result.Fields.ACTIVE_TASK, ignoreCase = true) -> {
                            // Closing of <active_task>
                            mResult.isActiveTask = true
                            mInActiveTask = false
                        }
                        localName.equals(Result.Fields.ACTIVE_TASK_STATE, ignoreCase = true) -> {
                            mResult.activeTaskState = mCurrentElement.toInt()
                        }
                        localName.equals(Result.Fields.APP_VERSION_NUM, ignoreCase = true) -> {
                            mResult.appVersionNum = mCurrentElement.toInt()
                        }
                        localName.equals(Result.Fields.SCHEDULER_STATE, ignoreCase = true) -> {
                            mResult.schedulerState = mCurrentElement.toInt()
                        }
                        localName.equals(Result.Fields.CHECKPOINT_CPU_TIME, ignoreCase = true) -> {
                            mResult.checkpointCPUTime = mCurrentElement.toDouble()
                        }
                        localName.equals(Result.Fields.CURRENT_CPU_TIME, ignoreCase = true) -> {
                            mResult.currentCPUTime = mCurrentElement.toDouble()
                        }
                        localName.equals(Result.Fields.FRACTION_DONE, ignoreCase = true) -> {
                            mResult.fractionDone = mCurrentElement.toFloat()
                        }
                        localName.equals(Result.Fields.ELAPSED_TIME, ignoreCase = true) -> {
                            mResult.elapsedTime = mCurrentElement.toDouble()
                        }
                        localName.equals(Result.Fields.SWAP_SIZE, ignoreCase = true) -> {
                            mResult.swapSize = mCurrentElement.toDouble()
                        }
                        localName.equals(Result.Fields.WORKING_SET_SIZE_SMOOTHED, ignoreCase = true) -> {
                            mResult.workingSetSizeSmoothed = mCurrentElement.toDouble()
                        }
                        localName.equals(Result.Fields.ESTIMATED_CPU_TIME_REMAINING, ignoreCase = true) -> {
                            mResult.estimatedCPUTimeRemaining = mCurrentElement.toDouble()
                        }
                        localName.equals(Result.Fields.SUPPORTS_GRAPHICS, ignoreCase = true) -> {
                            mResult.supportsGraphics = mCurrentElement.toString() != "0"
                        }
                        localName.equals(Result.Fields.GRAPHICS_MODE_ACKED, ignoreCase = true) -> {
                            mResult.graphicsModeAcked = mCurrentElement.toInt()
                        }
                        localName.equals(Result.Fields.TOO_LARGE, ignoreCase = true) -> {
                            mResult.isTooLarge = mCurrentElement.toString() != "0"
                        }
                        localName.equals(Result.Fields.NEEDS_SHMEM, ignoreCase = true) -> {
                            mResult.needsShmem = mCurrentElement.toString() != "0"
                        }
                        localName.equals(Result.Fields.EDF_SCHEDULED, ignoreCase = true) -> {
                            mResult.isEdfScheduled = mCurrentElement.toString() != "0"
                        }
                        localName.equals(Result.Fields.PID, ignoreCase = true) -> {
                            mResult.pid = mCurrentElement.toInt()
                        }
                        localName.equals(Result.Fields.SLOT, ignoreCase = true) -> {
                            mResult.slot = mCurrentElement.toInt()
                        }
                        localName.equals(Result.Fields.GRAPHICS_EXEC_PATH, ignoreCase = true) -> {
                            mResult.graphicsExecPath = mCurrentElement.toString()
                        }
                        localName.equals(Result.Fields.SLOT_PATH, ignoreCase = true) -> {
                            mResult.slotPath = mCurrentElement.toString()
                        }
                    }
                } else { // Not in <active_task>
                    when {
                        localName.equals(NAME, ignoreCase = true) -> {
                            mResult.name = mCurrentElement.toString()
                        }
                        localName.equals(Result.Fields.WU_NAME, ignoreCase = true) -> {
                            mResult.workUnitName = mCurrentElement.toString()
                        }
                        localName.equals(PROJECT_URL, ignoreCase = true) -> {
                            mResult.projectURL = mCurrentElement.toString()
                        }
                        localName.equals(Result.Fields.VERSION_NUM, ignoreCase = true) -> {
                            mResult.versionNum = mCurrentElement.toInt()
                        }
                        localName.equals(Result.Fields.READY_TO_REPORT, ignoreCase = true) -> {
                            mResult.isReadyToReport = mCurrentElement.toString() != "0"
                        }
                        localName.equals(Result.Fields.GOT_SERVER_ACK, ignoreCase = true) -> {
                            mResult.gotServerAck = mCurrentElement.toString() != "0"
                        }
                        localName.equals(Result.Fields.FINAL_CPU_TIME, ignoreCase = true) -> {
                            mResult.finalCPUTime = mCurrentElement.toDouble()
                        }
                        localName.equals(Result.Fields.FINAL_ELAPSED_TIME, ignoreCase = true) -> {
                            mResult.finalElapsedTime = mCurrentElement.toDouble()
                        }
                        localName.equals(Result.Fields.STATE, ignoreCase = true) -> {
                            mResult.state = mCurrentElement.toInt()
                        }
                        localName.equals(Result.Fields.REPORT_DEADLINE, ignoreCase = true) -> {
                            mResult.reportDeadline = mCurrentElement.toDouble().toLong()
                        }
                        localName.equals(Result.Fields.RECEIVED_TIME, ignoreCase = true) -> {
                            mResult.receivedTime = mCurrentElement.toDouble().toLong()
                        }
                        localName.equals(Result.Fields.ESTIMATED_CPU_TIME_REMAINING, ignoreCase = true) -> {
                            mResult.estimatedCPUTimeRemaining = mCurrentElement.toDouble()
                        }
                        localName.equals(Result.Fields.EXIT_STATUS, ignoreCase = true) -> {
                            mResult.exitStatus = mCurrentElement.toInt()
                        }
                        localName.equals(Result.Fields.SUSPENDED_VIA_GUI, ignoreCase = true) -> {
                            mResult.isSuspendedViaGUI = mCurrentElement.toString() != "0"
                        }
                        localName.equals(Result.Fields.PROJECT_SUSPENDED_VIA_GUI, ignoreCase = true) -> {
                            mResult.isProjectSuspendedViaGUI = mCurrentElement.toString() != "0"
                        }
                        localName.equals(Result.Fields.RESOURCES, ignoreCase = true) -> {
                            mResult.resources = mCurrentElement.toString()
                        }
                    }
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "ResultsParser.endElement error: ", e)
        }
        mElementStarted = false
    }

    companion object {
        const val RESULT_TAG = "result"
        /**
         * Parse the RPC result (results) and generate vector of results info
         *
         * @param rpcResult String returned by RPC call of core client
         * @return vector of results info
         */
        @JvmStatic
        fun parse(rpcResult: String?): List<Result> {
            return try {
                val parser = ResultsParser()
                Xml.parse(rpcResult, parser)
                parser.results
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "ResultsParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "ResultsParser: $rpcResult")

                emptyList()
            }
        }
    }
}
