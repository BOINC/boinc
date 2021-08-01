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

class CcStateParser : BaseParser() {
    val ccState = CcState()
    private var myProject = Project()
    private val mVersionInfo = VersionInfo()
    private val mAppsParser = AppsParser()
    private val mAppVersionsParser = AppVersionsParser()
    private val mHostInfoParser = HostInfoParser()
    private val mProjectsParser = ProjectsParser()
    private val mResultsParser = ResultsParser()
    private val mWorkUnitsParser = WorkUnitsParser()
    private var mInApp = false
    private var mInAppVersion = false
    private var mInHostInfo = false
    private var mInProject = false
    private var mInResult = false
    private var mInWorkUnit = false

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(CLIENT_STATE_TAG, ignoreCase = true)) {
            //Starting the query, clear mCcState
            ccState.clearArrays()
        }
        if (localName.equals(HostInfoParser.HOST_INFO_TAG, ignoreCase = true)) {
            // Just stepped inside <host_info>
            mInHostInfo = true
        }
        if (mInHostInfo) {
            mHostInfoParser.startElement(uri, localName, qName, attributes)
        }
        if (localName.equals(PROJECT, ignoreCase = true)) {
            // Just stepped inside <project>
            mInProject = true
        }
        if (mInProject) {
            mProjectsParser.startElement(uri, localName, qName, attributes)
        }
        if (localName.equals(AppsParser.APP_TAG, ignoreCase = true)) {
            // Just stepped inside <app>
            mInApp = true
        }
        if (mInApp) {
            mAppsParser.startElement(uri, localName, qName, attributes)
        }
        if (localName.equals(AppVersionsParser.APP_VERSION_TAG, ignoreCase = true)) {
            // Just stepped inside <app_version>
            mInAppVersion = true
        }
        if (mInAppVersion) {
            mAppVersionsParser.startElement(uri, localName, qName, attributes)
        }
        if (localName.equals(WorkUnitsParser.WORKUNIT_TAG, ignoreCase = true)) {
            // Just stepped inside <workunit>
            mInWorkUnit = true
        }
        if (mInWorkUnit) {
            mWorkUnitsParser.startElement(uri, localName, qName, attributes)
        }
        if (localName.equals(ResultsParser.RESULT_TAG, ignoreCase = true)) {
            // Just stepped inside <result>
            mInResult = true
        }
        if (mInResult) {
            mResultsParser.startElement(uri, localName, qName, attributes)
        }
        if (localName.equalsAny(CORE_CLIENT_MAJOR_VERSION_TAG, CORE_CLIENT_MINOR_VERSION_TAG,
                        CORE_CLIENT_RELEASE_TAG, CcState.Fields.HAVE_ATI,
                        CcState.Fields.HAVE_CUDA, ignoreCase = true)) {
            // VersionInfo elements
            mElementStarted = true
            mCurrentElement.setLength(0)
        }
    }

    @Throws(SAXException::class)
    override fun characters(ch: CharArray, start: Int, length: Int) {
        super.characters(ch, start, length)
        if (mInHostInfo) { // We are inside <host_info>
            mHostInfoParser.characters(ch, start, length)
        }
        if (mInProject) { // We are inside <project>
            mProjectsParser.characters(ch, start, length)
        }
        if (mInApp) { // We are inside <project>
            mAppsParser.characters(ch, start, length)
        }
        if (mInAppVersion) { // We are inside <project>
            mAppVersionsParser.characters(ch, start, length)
        }
        if (mInWorkUnit) { // We are inside <workunit>
            mWorkUnitsParser.characters(ch, start, length)
        }
        if (mInResult) { // We are inside <result>
            mResultsParser.characters(ch, start, length)
        }
        // VersionInfo elements are handled in super.characters()
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            if (mInHostInfo) { // We are inside <host_info>
                // parse it by sub-parser in any case (to parse also closing element)
                mHostInfoParser.endElement(uri, localName, qName)
                ccState.hostInfo = mHostInfoParser.hostInfo
                if (localName.equals(HostInfoParser.HOST_INFO_TAG, ignoreCase = true)) {
                    mInHostInfo = false
                }
            }
            if (mInProject) { // We are inside <project>
                // parse it by sub-parser in any case (must parse also closing element!)
                mProjectsParser.endElement(uri, localName, qName)
                if (localName.equals(PROJECT, ignoreCase = true)) {
                    // Closing tag of <project>
                    mInProject = false
                    val projects = mProjectsParser.projects
                    if (projects.isNotEmpty()) {
                        myProject = projects.last()
                        ccState.projects.add(myProject)
                    }
                }
            }
            if (mInApp) { // We are inside <app>
                // parse it by sub-parser in any case (must parse also closing element!)
                mAppsParser.endElement(uri, localName, qName)
                if (localName.equals(AppsParser.APP_TAG, ignoreCase = true)) {
                    // Closing tag of <app>
                    mInApp = false
                    val apps: List<App> = mAppsParser.apps
                    if (apps.isNotEmpty()) {
                        val myApp = apps.last()
                        myApp.project = myProject
                        ccState.apps.add(myApp)
                    }
                }
            }
            if (mInAppVersion) { // We are inside <app_version>
                // parse it by sub-parser in any case (must also parse closing element!)
                mAppVersionsParser.endElement(uri, localName, qName)
                if (localName.equals(AppVersionsParser.APP_VERSION_TAG, ignoreCase = true)) {
                    // Closing tag of <app_version>
                    mInAppVersion = false
                    val appVersions: List<AppVersion> = mAppVersionsParser.appVersions
                    if (appVersions.isNotEmpty()) {
                        val myAppVersion = appVersions.last()
                        myAppVersion.project = myProject
                        myAppVersion.app = ccState.lookupApp(myProject, myAppVersion.appName)
                        ccState.appVersions.add(myAppVersion)
                    }
                }
            }
            if (mInWorkUnit) { // We are inside <workunit>
                // parse it by sub-parser in any case (must parse also closing element!)
                mWorkUnitsParser.endElement(uri, localName, qName)
                if (localName.equals(WorkUnitsParser.WORKUNIT_TAG, ignoreCase = true)) {
                    // Closing tag of <workunit>
                    mInWorkUnit = false
                    val workUnits = mWorkUnitsParser.workUnits
                    if (workUnits.isNotEmpty()) {
                        val myWorkUnit = workUnits.last()
                        myWorkUnit.project = myProject
                        myWorkUnit.app = ccState.lookupApp(myProject, myWorkUnit.appName)
                        ccState.workUnits.add(myWorkUnit)
                    }
                }
            }
            if (mInResult) {
                // We are inside <result>
                // parse it by sub-parser in any case (must parse also closing element!)
                mResultsParser.endElement(uri, localName, qName)
                if (localName.equals(ResultsParser.RESULT_TAG, ignoreCase = true)) {
                    // Closing tag of <result>
                    mInResult = false
                    val results = mResultsParser.results
                    if (results.isNotEmpty()) {
                        val myResult = results.last()
                        myResult.project = myProject
                        myResult.workUnit = ccState.lookupWorkUnit(myProject, myResult.workUnitName)
                        if (myResult.workUnit != null) {
                            myResult.app = myResult.workUnit!!.app
                            myResult.appVersion = ccState.lookupAppVersion(myProject, myResult.app,
                                    myResult.versionNum, myResult.planClass)
                        }
                        ccState.results.add(myResult)
                    }
                }
            }
            if (mElementStarted) {
                trimEnd()
                // VersionInfo?
                when {
                    localName.equals(CORE_CLIENT_MAJOR_VERSION_TAG, ignoreCase = true) -> {
                        mVersionInfo.major = mCurrentElement.toInt()
                    }
                    localName.equals(CORE_CLIENT_MINOR_VERSION_TAG, ignoreCase = true) -> {
                        mVersionInfo.minor = mCurrentElement.toInt()
                    }
                    localName.equals(CORE_CLIENT_RELEASE_TAG, ignoreCase = true) -> {
                        mVersionInfo.release = mCurrentElement.toInt()
                    }
                    localName.equals(CcState.Fields.HAVE_ATI, ignoreCase = true) -> {
                        ccState.haveAti = mCurrentElement.toString() != "0"
                    }
                    localName.equals(CcState.Fields.HAVE_CUDA, ignoreCase = true) -> {
                        ccState.haveCuda = mCurrentElement.toString() != "0"
                    }
                }
                mElementStarted = false
                ccState.versionInfo = mVersionInfo
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "CcStateParser.endElement error: ", e)
        }
    }

    companion object {
        const val CLIENT_STATE_TAG = "client_state"
        const val CORE_CLIENT_MAJOR_VERSION_TAG = "core_client_major_version"
        const val CORE_CLIENT_MINOR_VERSION_TAG = "core_client_minor_version"
        const val CORE_CLIENT_RELEASE_TAG = "core_client_release"
        /**
         * Parse the RPC result (state) and generate vector of projects info
         *
         * @param rpcResult String returned by RPC call of core client
         * @return connected client state
         */
        @JvmStatic
        fun parse(rpcResult: String?): CcState? {
            return try {
                val parser = CcStateParser()
                Xml.parse(rpcResult, parser)
                parser.ccState
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "CcStateParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "CcStateParser: $rpcResult")

                null
            }
        }
    }
}
