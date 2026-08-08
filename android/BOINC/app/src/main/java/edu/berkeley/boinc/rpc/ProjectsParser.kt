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

class ProjectsParser : BaseParser() {
    val projects: MutableList<Project> = mutableListOf()
    private lateinit var mProject: Project
    private var mGuiUrl: GuiUrl? = null

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        when {
            localName.equals(PROJECT, ignoreCase = true) && !this::mProject.isInitialized -> {
                mProject = Project()
            }
            localName.equals(GUI_URL, ignoreCase = true) -> {
                mGuiUrl = GuiUrl()
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
            if (localName.equals(PROJECT, ignoreCase = true)) { // Closing tag of <project> - add to vector and be ready for next one
                if (mProject.masterURL.isNotEmpty()) { // master_url is a must
                    projects.add(mProject)
                }
                mProject = Project()
            } else { // Not the closing tag - we decode possible inner tags
                trimEnd()
                if (mGuiUrl != null) { // We are inside <gui_url> element
                    if (localName.equals(GUI_URL, ignoreCase = true)) { // finish of this <gui_url> element
                        mProject.guiURLs.add(mGuiUrl)
                        mGuiUrl = null
                    } else {
                        when {
                            localName.equals(NAME, ignoreCase = true) -> {
                                mGuiUrl!!.name = mCurrentElement.toString()
                            }
                            localName.equals(DESCRIPTION, ignoreCase = true) -> {
                                mGuiUrl!!.description = mCurrentElement.toString()
                            }
                            localName.equals(URL, ignoreCase = true) -> {
                                mGuiUrl!!.url = mCurrentElement.toString()
                            }
                        }
                    }
                } else if (localName.equals(MASTER_URL, ignoreCase = true)) {
                    mProject.masterURL = mCurrentElement.toString()
                } else if (localName.equals(Project.Fields.PROJECT_DIR, ignoreCase = true)) {
                    mProject.projectDir = mCurrentElement.toString()
                } else if (localName.equals(Project.Fields.RESOURCE_SHARE, ignoreCase = true)) {
                    mProject.resourceShare = mCurrentElement.toFloat()
                } else if (localName.equals(PROJECT_NAME, ignoreCase = true)) {
                    mProject.projectName = mCurrentElement.toString()
                } else if (localName.equals(Project.Fields.USER_NAME, ignoreCase = true)) {
                    mProject.userName = mCurrentElement.toString()
                } else if (localName.equals(Project.Fields.TEAM_NAME, ignoreCase = true)) {
                    mProject.teamName = mCurrentElement.toString()
                } else if (localName.equals(Project.Fields.HOSTID, ignoreCase = true)) {
                    mProject.hostId = mCurrentElement.toInt()
                } else if (localName.equals(Project.Fields.HOST_VENUE, ignoreCase = true)) {
                    mProject.hostVenue = mCurrentElement.toString()
                } else if (localName.equals(Project.Fields.USER_TOTAL_CREDIT, ignoreCase = true)) {
                    mProject.userTotalCredit = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.USER_EXPAVG_CREDIT, ignoreCase = true)) {
                    mProject.userExpAvgCredit = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.HOST_TOTAL_CREDIT, ignoreCase = true)) {
                    mProject.hostTotalCredit = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.HOST_EXPAVG_CREDIT, ignoreCase = true)) {
                    mProject.hostExpAvgCredit = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.NRPC_FAILURES, ignoreCase = true)) {
                    mProject.noOfRPCFailures = mCurrentElement.toInt()
                } else if (localName.equals(Project.Fields.MASTER_FETCH_FAILURES, ignoreCase = true)) {
                    mProject.masterFetchFailures = mCurrentElement.toInt()
                } else if (localName.equals(Project.Fields.MIN_RPC_TIME, ignoreCase = true)) {
                    mProject.minRPCTime = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.DOWNLOAD_BACKOFF, ignoreCase = true)) {
                    mProject.downloadBackoff = mCurrentElement.toString().toDouble()
                } else if (localName.equals(Project.Fields.UPLOAD_BACKOFF, ignoreCase = true)) {
                    mProject.uploadBackoff = mCurrentElement.toString().toDouble()
                } else if (localName.equals(SHORT_TERM_DEBT_TAG, ignoreCase = true)) {
                    mProject.cpuShortTermDebt = mCurrentElement.toString().toDouble()
                } else if (localName.equals(LONG_TERM_DEBT_TAG, ignoreCase = true)) {
                    mProject.cpuLongTermDebt = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.CPU_BACKOFF_TIME, ignoreCase = true)) {
                    mProject.cpuBackoffTime = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.CPU_BACKOFF_INTERVAL, ignoreCase = true)) {
                    mProject.cpuBackoffInterval = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.CUDA_DEBT, ignoreCase = true)) {
                    mProject.cudaDebt = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.CUDA_SHORT_TERM_DEBT, ignoreCase = true)) {
                    mProject.cudaShortTermDebt = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.CUDA_BACKOFF_TIME, ignoreCase = true)) {
                    mProject.cudaBackoffTime = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.CUDA_BACKOFF_INTERVAL, ignoreCase = true)) {
                    mProject.cudaBackoffInterval = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.ATI_DEBT, ignoreCase = true)) {
                    mProject.atiDebt = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.ATI_SHORT_TERM_DEBT, ignoreCase = true)) {
                    mProject.atiShortTermDebt = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.ATI_BACKOFF_TIME, ignoreCase = true)) {
                    mProject.atiBackoffTime = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.ATI_BACKOFF_INTERVAL, ignoreCase = true)) {
                    mProject.atiBackoffInterval = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.DURATION_CORRECTION_FACTOR, ignoreCase = true)) {
                    mProject.durationCorrectionFactor = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.MASTER_URL_FETCH_PENDING, ignoreCase = true)) {
                    mProject.masterURLFetchPending = mCurrentElement.toString() != "0"
                } else if (localName.equals(Project.Fields.SCHED_RPC_PENDING, ignoreCase = true)) {
                    mProject.scheduledRPCPending = mCurrentElement.toInt()
                } else if (localName.equals(NON_CPU_INTENSIVE, ignoreCase = true)) {
                    mProject.nonCPUIntensive = mCurrentElement.toString() != "0"
                } else if (localName.equals(Project.Fields.SUSPENDED_VIA_GUI, ignoreCase = true)) {
                    mProject.suspendedViaGUI = mCurrentElement.toString() != "0"
                } else if (localName.equals(Project.Fields.DONT_REQUEST_MORE_WORK, ignoreCase = true)) {
                    mProject.doNotRequestMoreWork = mCurrentElement.toString() != "0"
                } else if (localName.equals(Project.Fields.SCHEDULER_RPC_IN_PROGRESS, ignoreCase = true)) {
                    mProject.schedulerRPCInProgress = mCurrentElement.toString() != "0"
                } else if (localName.equals(Project.Fields.ATTACHED_VIA_ACCT_MGR, ignoreCase = true)) {
                    mProject.attachedViaAcctMgr = mCurrentElement.toString() != "0"
                } else if (localName.equals(Project.Fields.DETACH_WHEN_DONE, ignoreCase = true)) {
                    mProject.detachWhenDone = mCurrentElement.toString() != "0"
                } else if (localName.equals(Project.Fields.ENDED, ignoreCase = true)) {
                    mProject.ended = mCurrentElement.toString() != "0"
                } else if (localName.equals(Project.Fields.TRICKLE_UP_PENDING, ignoreCase = true)) {
                    mProject.trickleUpPending = mCurrentElement.toString() != "0"
                } else if (localName.equals(Project.Fields.PROJECT_FILES_DOWNLOADED_TIME, ignoreCase = true)) {
                    mProject.projectFilesDownloadedTime = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.LAST_RPC_TIME, ignoreCase = true)) {
                    mProject.lastRPCTime = mCurrentElement.toDouble()
                } else if (localName.equals(Project.Fields.NO_CPU_PREF, ignoreCase = true)) {
                    mProject.noCPUPref = mCurrentElement.toString() != "0"
                } else if (localName.equals(Project.Fields.NO_CUDA_PREF, ignoreCase = true)) {
                    mProject.noCUDAPref = mCurrentElement.toString() != "0"
                } else if (localName.equals(Project.Fields.NO_ATI_PREF, ignoreCase = true)) {
                    mProject.noATIPref = mCurrentElement.toString() != "0"
                } else if (localName.equals(Project.Fields.DISK_USAGE, ignoreCase = true)) {
                    mProject.diskUsage = mCurrentElement.toDouble()
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "ProjectsParser.endElement error: ", e)
        }
        mElementStarted = false
    }

    companion object {
        const val SHORT_TERM_DEBT_TAG = "short_term_debt"
        const val LONG_TERM_DEBT_TAG = "long_term_debt"
        /**
         * Parse the RPC result (projects) and generate vector of projects info
         *
         * @param rpcResult String returned by RPC call of core client
         * @return vector of projects info
         */
        @JvmStatic
        fun parse(rpcResult: String?): List<Project> {
            return try {
                val parser = ProjectsParser()
                Xml.parse(rpcResult, parser)
                parser.projects
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "ProjectsParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "ProjectsParser: $rpcResult")

                emptyList()
            }
        }
    }
}
