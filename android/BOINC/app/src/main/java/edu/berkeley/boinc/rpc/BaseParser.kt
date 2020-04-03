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

import org.apache.commons.lang3.StringUtils
import org.xml.sax.SAXException
import org.xml.sax.helpers.DefaultHandler

open class BaseParser : DefaultHandler() {
    @JvmField
    protected var mCurrentElement = StringBuilder()
    @JvmField
    protected var mElementStarted = false

    @Throws(SAXException::class)
    override fun characters(ch: CharArray, start: Int, length: Int) {
        super.characters(ch, start, length)
        if (mElementStarted) { // put it into StringBuilder
            if (mCurrentElement.isEmpty()) { // still empty - trim leading whitespace characters and append
                mCurrentElement.append(String(ch).trimStart())
            } else { // Non-empty - add everything
                mCurrentElement.append(ch, start, length)
            }
        }
    }

    protected fun trimEnd() {
        // Trim trailing spaces
        val str = mCurrentElement.trimEnd()
        mCurrentElement.setLength(0)
        mCurrentElement.append(str)
    }
}
