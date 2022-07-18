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
package edu.berkeley.boinc.utils

import com.google.common.io.CharSource
import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Test
import java.io.Reader

const val THIS = "This"
const val STRING = "This is a string."

class BOINCUtilsTest {
    private var stringBuilder = StringBuilder(STRING)
    private var reader: Reader = CharSource.wrap(stringBuilder).openStream()

    @Test
    fun `ReadLineLimit() When Reader has empty string and limit is 1 then expect null`() {
        stringBuilder.setLength(0)
        Assertions.assertNull(reader.readLineLimit(1))
    }

    @Test
    fun `ReadLineLimit() When Reader has non empty string and limit is length of string then expect Reader string`() {
        Assertions.assertEquals(STRING, reader.readLineLimit(stringBuilder.length))
    }

    @Test
    fun `ReadLineLimit() When Reader has non empty string and limit is length of string + 1 then expect Reader string`() {
        Assertions.assertEquals(STRING, reader.readLineLimit(stringBuilder.length + 1))
    }

    @Test
    fun `ReadLineLimit() When Reader has string with new line and limit is length of string then expect string without new line`() {
        stringBuilder.setCharAt(4, '\n')
        Assertions.assertEquals(THIS, reader.readLineLimit(stringBuilder.length))
    }

    @Test
    fun `ReadLineLimit() When Reader has string with carriage return and limit is length of string then expect string without carriage return`() {
        stringBuilder.setCharAt(4, '\r')
        Assertions.assertEquals(THIS, reader.readLineLimit(stringBuilder.length))
    }
}
