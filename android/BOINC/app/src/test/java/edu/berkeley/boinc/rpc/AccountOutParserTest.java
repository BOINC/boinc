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

import android.util.Xml;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.xml.sax.SAXException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class AccountOutParserTest {
    private AccountOutParser accountOutParser;

    @Before
    public void setUp() {
        accountOutParser = new AccountOutParser();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectNullPointerException() {
        mockStatic(Xml.class);

        assertThrows(NullPointerException.class, () -> AccountOutParser.parse(null));
    }

    @Test
    public void testParse_whenRpcStringIsEmpty_thenExpectNull() {
        mockStatic(Xml.class);

        assertNull(AccountOutParser.parse(""));
    }

    @Test
    public void testParser_whenLocalNameIsNull_thenExpectNullAccountOutAndElementStarted()
            throws SAXException {
        accountOutParser.startElement(null, null, null, null);

        assertNull(accountOutParser.getAccountOut());
        assertTrue(accountOutParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectNullAccountOutAndElementStarted() throws SAXException {
        accountOutParser.startElement(null, "", null, null);

        assertNull(accountOutParser.getAccountOut());
        assertTrue(accountOutParser.mElementStarted);
    }

    @Test
    public void testParser_whenXmlDocumentIsEmpty_thenExpectNullAccountOutAndElementNotStarted() throws SAXException {
        accountOutParser.startElement(null, "<?xml", null, null);
        accountOutParser.endElement(null, "?>", null);

        assertNull(accountOutParser.getAccountOut());
        assertFalse(accountOutParser.mElementStarted);
    }

    @Test
    public void testParser_whenXmlDocumentHasErrorNum_thenExpectAccountOutWithErrorNum() throws SAXException {
        accountOutParser.startElement(null, "<?xml", null, null);
        accountOutParser.startElement(null, AccountOut.Fields.error_num, null, null);
        accountOutParser.characters("1".toCharArray(), 0, 1);
        accountOutParser.endElement(null, AccountOut.Fields.error_num, null);
        accountOutParser.endElement(null, "?>", null);

        final AccountOut expected = new AccountOut();
        expected.error_num = 1;

        assertEquals(expected, accountOutParser.getAccountOut());
    }

    @Test
    public void testParser_whenXmlDocumentHasErrorMessage_thenExpectAccountOutWithErrorMessage() throws SAXException {
        accountOutParser.startElement(null, "<?xml", null, null);
        accountOutParser.startElement(null, AccountOut.Fields.error_msg, null, null);
        accountOutParser.characters("Error message".toCharArray(), 0, 13);
        accountOutParser.endElement(null, AccountOut.Fields.error_msg, null);
        accountOutParser.endElement(null, "?>", null);

        final AccountOut expected = new AccountOut();
        expected.error_msg = "Error message";

        assertEquals(expected, accountOutParser.getAccountOut());
    }

    @Test
    public void testParser_whenXmlDocumentHasAuthenticator_thenExpectAccountOutWithAuthenticator() throws SAXException {
        accountOutParser.startElement(null, "<?xml", null, null);
        accountOutParser.startElement(null, AccountOut.Fields.authenticator, null, null);
        accountOutParser.characters("Authenticator".toCharArray(), 0, 13);
        accountOutParser.endElement(null, AccountOut.Fields.authenticator, null);
        accountOutParser.endElement(null, "?>", null);

        final AccountOut expected = new AccountOut();
        expected.authenticator = "Authenticator";

        assertEquals(expected, accountOutParser.getAccountOut());
    }
}