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
import org.junit.jupiter.api.BeforeEach
import org.junit.jupiter.api.Test
import java.io.Reader
import kotlin.test.junit5.JUnit5Asserter

const val THIS = "This"
const val STRING = "This is a string."

internal class BOINCUtilsTest {
    private lateinit var stringBuilder: StringBuilder
    private lateinit var reader: Reader

    @BeforeEach
    fun setUp() {
        stringBuilder = StringBuilder(STRING)
        reader = CharSequenceReader(stringBuilder)
    }

    @Test
    fun testReadLineLimit_whenReaderHasEmptyStringAndLimitIs1_thenExpectNull() {
        stringBuilder.setLength(0)
        JUnit5Asserter.assertNull("Expected null", reader.readLineLimit(1))
    }

    @Test
    fun testReadLineLimit_whenReaderHasNonEmptyStringAndLimitIsLengthOfString_thenExpectReaderString() {
        JUnit5Asserter.assertEquals("Expected $STRING", STRING, reader.readLineLimit(stringBuilder.length))
    }

    @Test
    fun testReadLineLimit_whenReaderHasNonEmptyStringAndLimitIsLengthOfStringPlus1_thenExpectReaderString() {
        JUnit5Asserter.assertEquals("Expected $STRING", STRING, reader.readLineLimit(stringBuilder.length + 1))
    }

    @Test
    fun testReadLineLimit_whenReaderHasStringWithNewlineAndLimitIsLengthOfString_thenExpectStringWithoutNewline() {
        stringBuilder.setCharAt(4, '\n')
        JUnit5Asserter.assertEquals("Expected $THIS", THIS, reader.readLineLimit(stringBuilder.length))
    }

    @Test
    fun testReadLineLimit_whenReaderHasStringWithCarriageReturnAndLimitIsLengthOfString_thenExpectStringWithoutCarriageReturn() {
        stringBuilder.setCharAt(4, '\r')
        JUnit5Asserter.assertEquals("Expected $THIS", THIS, reader.readLineLimit(stringBuilder.length))
    }
}
