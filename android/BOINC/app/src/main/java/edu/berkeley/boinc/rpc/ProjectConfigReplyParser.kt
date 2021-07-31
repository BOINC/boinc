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

class ProjectConfigReplyParser : BaseParser() {
    lateinit var projectConfig: ProjectConfig
        private set
    private var mPlatforms: MutableList<PlatformInfo?>? = null
    private var withinPlatforms = false
    private var platformName = ""
    private var platformFriendlyName = ""
    private var platformPlanClass = ""

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        when {
            localName.equals(PROJECT_CONFIG_TAG, ignoreCase = true) -> {
                projectConfig = ProjectConfig()
            }
            localName.equals(ProjectConfig.Fields.PLATFORMS, ignoreCase = true) -> {
                withinPlatforms = true
                mPlatforms = mutableListOf() //initialize new list (flushing old elements)
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
            if (localName.equals(ProjectConfig.Fields.PLATFORMS, ignoreCase = true)) { // closing tag of platform names
                projectConfig.platforms = mPlatforms!!
                withinPlatforms = false
            } else { // Not the closing tag - we decode possible inner tags
                trimEnd()
                if (localName.equals(NAME, ignoreCase = true)) {
                    projectConfig.name = mCurrentElement.toString()
                } else if (localName.equals(MASTER_URL, ignoreCase = true)) {
                    projectConfig.masterUrl = mCurrentElement.toString()
                } else if (localName.equals(ProjectConfig.Fields.WEB_RPC_URL_BASE, ignoreCase = true)) {
                    projectConfig.webRpcUrlBase = mCurrentElement.toString()
                } else if (localName.equals(ProjectConfig.Fields.LOCAL_REVISION, ignoreCase = true)) {
                    projectConfig.localRevision = mCurrentElement.toString()
                } else if (localName.equals(ProjectConfig.Fields.MIN_PWD_LENGTH, ignoreCase = true)) {
                    projectConfig.minPwdLength = mCurrentElement.toInt()
                } else if (localName.equalsAny(USER_NAME_TAG, USES_USER_NAME_TAG, ignoreCase = true)) {
                    projectConfig.usesName = true
                } else if (localName.equals(ProjectConfig.Fields.WEB_STOPPED, ignoreCase = true)) {
                    projectConfig.webStopped = mCurrentElement.toInt() != 0
                } else if (localName.equals(ProjectConfig.Fields.SCHEDULER_STOPPED, ignoreCase = true)) {
                    projectConfig.schedulerStopped = mCurrentElement.toInt() != 0
                } else if (localName.equals(ProjectConfig.Fields.CLIENT_ACCOUNT_CREATION_DISABLED,
                                ignoreCase = true)) {
                    projectConfig.clientAccountCreationDisabled = true
                } else if (localName.equals(ProjectConfig.Fields.MIN_CLIENT_VERSION, ignoreCase = true)) {
                    projectConfig.minClientVersion = mCurrentElement.toInt()
                } else if (localName.equals(ProjectConfig.Fields.RPC_PREFIX, ignoreCase = true)) {
                    projectConfig.rpcPrefix = mCurrentElement.toString()
                } else if (localName.equals(PlatformInfo.Fields.NAME, ignoreCase = true) &&
                        withinPlatforms) {
                    platformName = mCurrentElement.toString()
                } else if (localName.equals(USER_FRIENDLY_NAME, ignoreCase = true) && withinPlatforms) {
                    platformFriendlyName = mCurrentElement.toString()
                } else if (localName.equals(PLAN_CLASS, ignoreCase = true) && withinPlatforms) {
                    platformPlanClass = mCurrentElement.toString()
                } else if (localName.equals(ProjectConfig.Fields.TERMS_OF_USE, ignoreCase = true)) {
                    projectConfig.termsOfUse = mCurrentElement.toString()
                } else if (localName.equals(PLATFORM_TAG, ignoreCase = true) && withinPlatforms) {
                    // finish platform object and add to array
                    mPlatforms!!.add(PlatformInfo(platformName, platformFriendlyName,
                            platformPlanClass))
                    platformFriendlyName = ""
                    platformName = ""
                    platformPlanClass = ""
                } else if (localName.equals(ERROR_NUM, ignoreCase = true)) { // reply is not present yet
                    projectConfig.errorNum = mCurrentElement.toInt()
                } else if (localName.equals(ProjectConfig.Fields.ACCOUNT_MANAGER, ignoreCase = true)) {
                    projectConfig.accountManager = true
                }
            }
            mElementStarted = false
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "ProjectConfigReplyParser.endElement error: ", e)
        }
    }

    companion object {
        const val PLATFORM_TAG = "platform"
        const val PROJECT_CONFIG_TAG = "project_config"
        const val USER_NAME_TAG = "user_name"
        const val USES_USER_NAME_TAG = "uses_username"
        /**
         * Parse the RPC result (state) and generate vector of projects info
         *
         * @param rpcResult String returned by RPC call of core client
         * @return connected client state
         */
        @JvmStatic
        fun parse(rpcResult: String): ProjectConfig? {
            return try {
                val parser = ProjectConfigReplyParser()
                //TODO report malformated XML to BOINC and remove String.replace here...
                //<boinc_gui_rpc_reply>
                //<?xml version="1.0" encoding="ISO-8859-1" ?>
                //<project_config>
                Xml.parse(rpcResult.replace("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>", ""), parser)
                parser.projectConfig
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "ProjectConfigReplyParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "ProjectConfigReplyParser: $rpcResult")

                null
            }
        }
    }
}
