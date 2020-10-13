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
import org.xml.sax.ContentHandler;
import org.xml.sax.SAXException;

import kotlin.UninitializedPropertyAccessException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.doThrow;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class AccountOutParserTest {
    private AccountOutParser accountOutParser;
    private AccountOut expected;

    @Before
    public void setUp() {
        accountOutParser = new AccountOutParser();
        expected = new AccountOut();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectNullPointerException() {
        mockStatic(Xml.class);

        assertThrows(NullPointerException.class, () -> AccountOutParser.parse(null));
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParse_whenRpcStringIsEmpty_thenExpectUninitializedPropertyAccessException() {
        mockStatic(Xml.class);

        AccountOutParser.parse("");
    }

    @Test
    public void testParse_whenSAXExceptionIsThrown_thenExpectNull() throws Exception {
        mockStatic(Xml.class);

        doThrow(new SAXException()).when(Xml.class, "parse", anyString(), any(ContentHandler.class));

        assertNull(AccountOutParser.parse(""));
    }

    @Test(expected = NullPointerException.class)
    public void testParser_whenLocalNameIsNull_thenExpectNullPointerException()
            throws SAXException {
        accountOutParser.startElement(null, null, null, null);
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParser_whenLocalNameIsEmpty_thenExpectUninitializedPropertyAccessException() throws SAXException {
        accountOutParser.startElement(null, "", null, null);

        accountOutParser.getAccountOut();
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParser_whenXmlDocumentIsEmpty_thenExpectUninitializedPropertyAccessException() throws SAXException {
        accountOutParser.startElement(null, "<?xml", null, null);
        accountOutParser.endElement(null, "?>", null);

        accountOutParser.getAccountOut();
    }

    @Test
    public void testParser_whenXmlDocumentHasInvalidErrorNum_thenExpectDefaultAccountOut() throws SAXException {
        accountOutParser.startElement(null, "<?xml", null, null);
        accountOutParser.startElement(null, RPCCommonTags.ERROR_NUM, null, null);
        accountOutParser.characters("One".toCharArray(), 0, 3);
        accountOutParser.endElement(null, RPCCommonTags.ERROR_NUM, null);
        accountOutParser.endElement(null, "?>", null);

        assertEquals(expected, accountOutParser.getAccountOut());
    }

    @Test
    public void testParser_whenXmlDocumentHasErrorNum_thenExpectAccountOutWithErrorNum()
            throws SAXException {
        accountOutParser.startElement(null, "<?xml", null, null);
        accountOutParser.startElement(null, RPCCommonTags.ERROR_NUM, null, null);
        accountOutParser.characters("1".toCharArray(), 0, 1);
        accountOutParser.endElement(null, RPCCommonTags.ERROR_NUM, null);
        accountOutParser.endElement(null, "?>", null);

        expected.setErrorNum(1);

        assertEquals(expected, accountOutParser.getAccountOut());
    }

    @Test
    public void testParser_whenXmlDocumentHasErrorMessage_thenExpectAccountOutWithErrorMessage() throws SAXException {
        accountOutParser.startElement(null, "<?xml", null, null);
        accountOutParser.startElement(null, AccountOut.Fields.ERROR_MSG, null, null);
        accountOutParser.characters("Error message".toCharArray(), 0, 13);
        accountOutParser.endElement(null, AccountOut.Fields.ERROR_MSG, null);
        accountOutParser.endElement(null, "?>", null);

        expected.setErrorMsg("Error message");

        assertEquals(expected, accountOutParser.getAccountOut());
    }

    @Test
    public void testParser_whenXmlDocumentHasAuthenticator_thenExpectAccountOutWithAuthenticator() throws SAXException {
        accountOutParser.startElement(null, "<?xml", null, null);
        accountOutParser.startElement(null, AccountOut.Fields.AUTHENTICATOR, null, null);
        accountOutParser.characters("Authenticator".toCharArray(), 0, 13);
        accountOutParser.endElement(null, AccountOut.Fields.AUTHENTICATOR, null);
        accountOutParser.endElement(null, "?>", null);

        expected.setAuthenticator("Authenticator");

        assertEquals(expected, accountOutParser.getAccountOut());
    }
}
