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

import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.BeforeEach
import org.junit.jupiter.api.Test
import org.xml.sax.SAXException

internal class BaseParserTest {
    private lateinit var baseParser: BaseParser
    @BeforeEach
    fun setUp() {
        baseParser = BaseParser()
    }

    @Test
    @Throws(SAXException::class)
    fun `When buffer is empty and string has no leading whitespace then expect identical string to input`() {
        baseParser.mElementStarted = true
        baseParser.characters(TEST_STRING.toCharArray(), 0, TEST_STRING.length)
        Assertions.assertEquals(TEST_STRING, baseParser.mCurrentElement.toString())
    }

    @Test
    @Throws(SAXException::class)
    fun `When buffer is empty and string has leading whitespace then expect string without leading whitespace`() {
        baseParser.mElementStarted = true
        baseParser.characters((" $TEST_STRING").toCharArray(), 0, TEST_STRING.length + 1)
        Assertions.assertEquals(TEST_STRING, baseParser.mCurrentElement.toString())
    }

    @Test
    @Throws(SAXException::class)
    fun `When buffer is not empty and string has no leading whitespace then expect concatenated string`() {
        baseParser.mCurrentElement.append(TEST_STRING)
        baseParser.mElementStarted = true
        baseParser.characters(TEST_STRING.toCharArray(), 0, TEST_STRING.length)
        Assertions.assertEquals(TEST_STRING + TEST_STRING, baseParser.mCurrentElement.toString())
    }

    @Test
    @Throws(SAXException::class)
    fun `When buffer is not empty and string has leading whitespace then expect concatenated string separated by space`() {
        baseParser.mCurrentElement.append(TEST_STRING)
        baseParser.mElementStarted = true
        baseParser.characters((" $TEST_STRING").toCharArray(), 0, TEST_STRING.length + 1)
        Assertions.assertEquals(
            "$TEST_STRING $TEST_STRING",
            baseParser.mCurrentElement.toString()
        )
    }

    @Test
    fun `When string has no trailing whitespace then expect identical string to input`() {
        baseParser.mCurrentElement.append(TEST_STRING)
        baseParser.trimEnd()
        Assertions.assertEquals(TEST_STRING, baseParser.mCurrentElement.toString())
    }

    @Test
    fun `When string has trailing whitespace then expect string without trailing whitespace`() {
        baseParser.mCurrentElement.append("$TEST_STRING ")
        baseParser.trimEnd()
        Assertions.assertEquals(TEST_STRING, baseParser.mCurrentElement.toString())
    }

    companion object {
        private const val TEST_STRING = "This is a test string."
    }
}
