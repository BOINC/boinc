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
package edu.berkeley.boinc.utils

import org.apache.commons.io.input.CharSequenceReader
import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Test
import java.io.Reader

const val THIS = "This"
const val STRING = "This is a string."

class BOINCUtilsTest {
    private var stringBuilder = StringBuilder(STRING)
    private var reader: Reader = CharSequenceReader(stringBuilder)

    @Test
    fun testReadLineLimit_whenReaderHasEmptyStringAndLimitIs1_thenExpectNull() {
        stringBuilder.setLength(0)
        Assertions.assertNull(reader.readLineLimit(1))
    }

    @Test
    fun testReadLineLimit_whenReaderHasNonEmptyStringAndLimitIsLengthOfString_thenExpectReaderString() {
        Assertions.assertEquals(STRING, reader.readLineLimit(stringBuilder.length))
    }

    @Test
    fun testReadLineLimit_whenReaderHasNonEmptyStringAndLimitIsLengthOfStringPlus1_thenExpectReaderString() {
        Assertions.assertEquals(STRING, reader.readLineLimit(stringBuilder.length + 1))
    }

    @Test
    fun testReadLineLimit_whenReaderHasStringWithNewlineAndLimitIsLengthOfString_thenExpectStringWithoutNewline() {
        stringBuilder.setCharAt(4, '\n')
        Assertions.assertEquals(THIS, reader.readLineLimit(stringBuilder.length))
    }

    @Test
    fun testReadLineLimit_whenReaderHasStringWithCarriageReturnAndLimitIsLengthOfString_thenExpectStringWithoutCarriageReturn() {
        stringBuilder.setCharAt(4, '\r')
        Assertions.assertEquals(THIS, reader.readLineLimit(stringBuilder.length))
    }
}
