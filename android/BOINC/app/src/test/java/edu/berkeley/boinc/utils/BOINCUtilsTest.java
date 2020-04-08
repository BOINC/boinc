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
package edu.berkeley.boinc.utils;

import org.apache.commons.io.input.CharSequenceReader;
import org.junit.Before;
import org.junit.Test;

import java.io.IOException;
import java.io.Reader;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

public class BOINCUtilsTest {
    private StringBuilder stringBuilder;
    private Reader reader;

    @Before
    public void setUp() {
        stringBuilder = new StringBuilder("This is a string.");
        reader = new CharSequenceReader(stringBuilder);
    }

    @Test
    public void testReadLineLimit_whenReaderIsNullAndLimitIs0_thenExpectEmptyString()
            throws IOException {
        String result = BOINCUtils.readLineLimit(null, 0);

        assertNotNull(result);
        assertTrue(result.isEmpty());
    }

    @Test(expected = NullPointerException.class)
    public void testReadLineLimit_whenReaderIsNullAndLimitIs1_thenExpectNullPointerException()
            throws IOException {
        BOINCUtils.readLineLimit(null, 1);
    }

    @Test
    public void testReadLineLimit_whenReaderHasEmptyStringAndLimitIs1_thenExpectNull()
            throws IOException {
        stringBuilder.setLength(0);

        assertNull(BOINCUtils.readLineLimit(reader, 1));
    }

    @Test
    public void testReadLineLimit_whenReaderHasNonEmptyStringAndLimitIsLengthOfString_thenExpectReaderString()
            throws IOException {
        assertEquals(BOINCUtils.readLineLimit(reader, stringBuilder.length()), "This is a string.");
    }

    @Test
    public void testReadLineLimit_whenReaderHasNonEmptyStringAndLimitIsLengthOfStringPlus1_thenExpectReaderString()
            throws IOException {
        assertEquals(BOINCUtils.readLineLimit(reader, stringBuilder.length() + 1), "This is a string.");
    }

    @Test
    public void testReadLineLimit_whenReaderHasStringWithNewlineAndLimitIsLengthOfString_thenExpectStringWithoutNewline()
            throws IOException {
        stringBuilder.setCharAt(4, '\n');

        assertEquals(BOINCUtils.readLineLimit(reader, stringBuilder.length()), "This");
    }

    @Test
    public void testReadLineLimit_whenReaderHasStringWithCarriageReturnAndLimitIsLengthOfString_thenExpectStringWithoutCarriageReturn()
            throws IOException {
        stringBuilder.setCharAt(4, '\r');

        assertEquals(BOINCUtils.readLineLimit(reader, stringBuilder.length()), "This");
    }
}
