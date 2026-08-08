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

import androidx.annotation.VisibleForTesting
import org.xml.sax.SAXException
import org.xml.sax.helpers.DefaultHandler

open class BaseParser : DefaultHandler() {
    @JvmField
    @VisibleForTesting(otherwise = VisibleForTesting.PROTECTED)
    var mCurrentElement = StringBuilder()
    @JvmField
    @VisibleForTesting(otherwise = VisibleForTesting.PROTECTED)
    var mElementStarted = false

    @Throws(SAXException::class)
    override fun characters(ch: CharArray, start: Int, length: Int) {
        super.characters(ch, start, length)
        if (mElementStarted) { // put it into StringBuilder
            if (mCurrentElement.isEmpty()) {
                // still empty - trim leading white-spaces
                var newStart = start
                var newLength = length
                while (newLength > 0) {
                    if (!ch[newStart].isWhitespace()) {
                        // First non-white-space character
                        mCurrentElement.append(ch, newStart, newLength)
                        break
                    }
                    ++newStart
                    --newLength
                }
            } else { // Non-empty - add everything
                mCurrentElement.append(ch, start, length)
            }
        }
    }

    @VisibleForTesting(otherwise = VisibleForTesting.PROTECTED)
    fun trimEnd() {
        val length = mCurrentElement.length
        // Trim trailing spaces
        for (i in length - 1 downTo 0) {
            if (!mCurrentElement[i].isWhitespace()) {
                // All trailing white-spaces are skipped, i is position of last character
                mCurrentElement.setLength(i + 1)
                break
            }
        }
    }
}
