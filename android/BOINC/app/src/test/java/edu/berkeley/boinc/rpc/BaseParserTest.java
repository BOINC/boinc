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
package edu.berkeley.boinc.rpc;

import org.junit.Before;
import org.junit.Test;
import org.xml.sax.SAXException;

import static org.junit.Assert.*;

public class BaseParserTest {
    private static final String TEST_STRING = "This is a test string.";

    private BaseParser baseParser;

    @Before
    public void setUp() {
        baseParser = new BaseParser();
    }

    @Test
    public void testCharacters_whenBufferIsEmptyAndStringHasNoLeadingWhitespace_thenExpectIdenticalStringToInput()
            throws SAXException {
        baseParser.mElementStarted = true;
        baseParser.characters(TEST_STRING.toCharArray(), 0, TEST_STRING.length());

        assertEquals(TEST_STRING, baseParser.mCurrentElement.toString());
    }

    @Test
    public void testCharacters_whenBufferIsEmptyAndStringHasLeadingWhitespace_thenExpectStringWithoutLeadingWhitespace()
            throws SAXException {
        baseParser.mElementStarted = true;
        baseParser.characters((" " + TEST_STRING).toCharArray(), 0, TEST_STRING.length() + 1);

        assertEquals(TEST_STRING, baseParser.mCurrentElement.toString());
    }

    @Test
    public void testCharacters_whenBufferIsNotEmptyAndStringHasNoLeadingWhitespace_thenExpectConcatenatedString()
            throws SAXException {
        baseParser.mCurrentElement.append(TEST_STRING);
        baseParser.mElementStarted = true;
        baseParser.characters(TEST_STRING.toCharArray(), 0, TEST_STRING.length());

        assertEquals(TEST_STRING + TEST_STRING, baseParser.mCurrentElement.toString());
    }

    @Test
    public void testCharacters_whenBufferIsNotEmptyAndStringHasLeadingWhitespace_thenExpectConcatenatedStringSeparatedBySpace()
            throws SAXException {
        baseParser.mCurrentElement.append(TEST_STRING);
        baseParser.mElementStarted = true;
        baseParser.characters((" " + TEST_STRING).toCharArray(), 0, TEST_STRING.length() + 1);

        assertEquals(TEST_STRING + " " + TEST_STRING, baseParser.mCurrentElement.toString());
    }

    @Test
    public void testTrimEnd_whenStringHasNoTrailingWhitespace_thenExpectIdenticalStringToInput() {
        baseParser.mCurrentElement.append(TEST_STRING);
        baseParser.trimEnd();

        assertEquals(TEST_STRING, baseParser.mCurrentElement.toString());
    }

    @Test
    public void testTrimEnd_whenStringHasTrailingWhitespace_thenExpectStringWithoutTrailingWhitespace() {
        baseParser.mCurrentElement.append(TEST_STRING + " ");
        baseParser.trimEnd();

        assertEquals(TEST_STRING, baseParser.mCurrentElement.toString());
    }
}
