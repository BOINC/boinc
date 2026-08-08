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

class ProjectInfoParser : BaseParser() {
    val projectInfos: MutableList<ProjectInfo> = ArrayList()
    private lateinit var mProjectInfo: ProjectInfo
    private lateinit var mPlatforms: MutableList<String>
    private var withinPlatforms = false

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        when {
            localName.equals(PROJECT, ignoreCase = true) -> mProjectInfo = ProjectInfo()
            localName.equals(ProjectInfo.Fields.PLATFORMS, ignoreCase = true) -> {
                mPlatforms = ArrayList() //initialize new list (flushing old elements)
                withinPlatforms = true
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
            if (this::mProjectInfo.isInitialized) {
                if (localName.equals(PROJECT, ignoreCase = true)) {
                    // Closing tag of <project> - add to list and be ready for next one
                    if (mProjectInfo.name.isNotEmpty()) {
                        // name is a must
                        projectInfos.add(mProjectInfo)
                    }
                    mProjectInfo = ProjectInfo()
                } else if (localName.equals(ProjectInfo.Fields.PLATFORMS, ignoreCase = true)) {
                    // closing tag of platform names
                    mProjectInfo.platforms = mPlatforms
                    withinPlatforms = false
                } else {
                    // Not the closing tag - we decode possible inner tags
                    trimEnd()
                    if (localName.equals(NAME, ignoreCase = true) && !withinPlatforms) {
                        //project name
                        mProjectInfo.name = mCurrentElement.toString()
                    } else if (localName.equals(URL, ignoreCase = true)) {
                        mProjectInfo.url = mCurrentElement.toString()
                    } else if (localName.equals(ProjectInfo.Fields.GENERAL_AREA, ignoreCase = true)) {
                        mProjectInfo.generalArea = mCurrentElement.toString()
                    } else if (localName.equals(ProjectInfo.Fields.SPECIFIC_AREA, ignoreCase = true)) {
                        mProjectInfo.specificArea = mCurrentElement.toString()
                    } else if (localName.equals(DESCRIPTION, ignoreCase = true)) {
                        mProjectInfo.description = mCurrentElement.toString()
                    } else if (localName.equals(ProjectInfo.Fields.HOME, ignoreCase = true)) {
                        mProjectInfo.home = mCurrentElement.toString()
                    } else if (localName.equals(NAME, ignoreCase = true) && withinPlatforms) {
                        //platform name
                        mPlatforms.add(mCurrentElement.toString())
                    } else if (localName.equals(ProjectInfo.Fields.IMAGE_URL, ignoreCase = true)) {
                        mProjectInfo.imageUrl = mCurrentElement.toString()
                    } else if (localName.equals(ProjectInfo.Fields.SUMMARY, ignoreCase = true)) {
                        mProjectInfo.summary = mCurrentElement.toString()
                    }
                }
            }
            mElementStarted = false
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "ProjectInfoParser.endElement error: ", e)
        }
    }

    companion object {
        @JvmStatic
        fun parse(rpcResult: String): List<ProjectInfo> {
            return try {
                val parser = ProjectInfoParser()
                // Uncomment if any issues arise
                // Xml.parse(rpcResult.replace("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>", ""), parser)
                Xml.parse(rpcResult, parser)
                parser.projectInfos
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "ProjectInfoParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "ProjectInfoParser: $rpcResult")

                emptyList()
            }
        }
    }
}
