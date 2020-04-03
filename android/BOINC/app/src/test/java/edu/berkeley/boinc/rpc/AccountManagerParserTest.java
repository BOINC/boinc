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

import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.doThrow;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class AccountManagerParserTest {
    private static final String ACCOUNT_MANAGER = "Account Manager";
    private static final String DESCRIPTION = "Description";
    private static final String IMAGE_URL = "Image URL";

    private AccountManagerParser accountManagerParser;
    private List<AccountManager> accountManagers;
    private AccountManager expected;

    @Before
    public void setUp() {
        accountManagerParser = new AccountManagerParser();
        accountManagers = accountManagerParser.getAccountManagerInfos();
        expected = new AccountManager();
    }

    @Test
    public void testParser_whenLocalNameIsNull_thenExpectIllegalArgumentExceptionAndEmptyList() {
        assertThrows(IllegalArgumentException.class, () ->
                accountManagerParser.startElement(null, null, null, null));

        assertTrue(accountManagers.isEmpty());
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectElementStarted() throws SAXException {
        accountManagerParser.startElement(null, "", null, null);

        assertTrue(accountManagerParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectEmptyList() throws SAXException {
        accountManagerParser.startElement(null, "", null, null);

        assertTrue(accountManagers.isEmpty());
    }

    @Test
    public void testParser_whenOneAccountManagerWithNoElements_thenExpectEmptyList() throws SAXException {
        accountManagerParser.startElement(null, RPCCommonTags.ACCOUNT_MANAGER, null, null);
        accountManagerParser.endElement(null, RPCCommonTags.ACCOUNT_MANAGER, null);

        assertTrue(accountManagers.isEmpty());
    }

    @Test
    public void testParser_whenXmlAccountManagerHasOnlyNameWithoutClosingTag_thenExpectElementNotStarted()
            throws SAXException {
        accountManagerParser.startElement(null, RPCCommonTags.ACCOUNT_MANAGER, null, null);
        accountManagerParser.startElement(null, RPCCommonTags.NAME, null, null);
        accountManagerParser.characters(ACCOUNT_MANAGER.toCharArray(), 0, ACCOUNT_MANAGER.length());
        accountManagerParser.endElement(null, "", null);
        accountManagerParser.endElement(null, RPCCommonTags.ACCOUNT_MANAGER, null);

        assertFalse(accountManagerParser.mElementStarted);
    }

    @Test
    public void testParser_whenXmlAccountManagerHasOnlyNameWithoutClosingTag_thenExpectEmptyList()
            throws SAXException {
        accountManagerParser.startElement(null, RPCCommonTags.ACCOUNT_MANAGER, null, null);
        accountManagerParser.startElement(null, RPCCommonTags.NAME, null, null);
        accountManagerParser.characters(ACCOUNT_MANAGER.toCharArray(), 0, ACCOUNT_MANAGER.length());
        accountManagerParser.endElement(null, "", null);
        accountManagerParser.endElement(null, RPCCommonTags.ACCOUNT_MANAGER, null);

        assertTrue(accountManagers.isEmpty());
    }

    @Test
    public void testParser_whenOneAccountManagerWithOnlyName_thenExpectElementWithOnlyName()
            throws SAXException {
        accountManagerParser.startElement(null, RPCCommonTags.ACCOUNT_MANAGER, null, null);
        accountManagerParser.startElement(null, RPCCommonTags.NAME, null, null);
        accountManagerParser.characters(ACCOUNT_MANAGER.toCharArray(), 0, ACCOUNT_MANAGER.length());
        accountManagerParser.endElement(null, RPCCommonTags.NAME, null);
        accountManagerParser.endElement(null, RPCCommonTags.ACCOUNT_MANAGER, null);

        expected.setName(ACCOUNT_MANAGER);

        assertEquals(1, accountManagers.size());
        assertEquals(expected, accountManagers.get(0));
    }

    @Test
    public void testParser_whenXmlAccountManagerHasOnlyNameAndUrl_thenExpectMatchingAccountManager()
            throws SAXException {
        accountManagerParser.startElement(null, RPCCommonTags.ACCOUNT_MANAGER, null, null);
        accountManagerParser.startElement(null, RPCCommonTags.NAME, null, null);
        accountManagerParser.characters(ACCOUNT_MANAGER.toCharArray(), 0, ACCOUNT_MANAGER.length());
        accountManagerParser.endElement(null, RPCCommonTags.NAME, null);
        accountManagerParser.startElement(null, RPCCommonTags.URL, null, null);
        accountManagerParser.characters("URL".toCharArray(), 0, 3);
        accountManagerParser.endElement(null, RPCCommonTags.URL, null);
        accountManagerParser.endElement(null, RPCCommonTags.ACCOUNT_MANAGER, null);

        expected.setName(ACCOUNT_MANAGER);
        expected.setUrl("URL");

        assertEquals(1, accountManagers.size());
        assertEquals(expected, accountManagers.get(0));
    }

    @Test
    public void testParser_whenXmlAccountManagerWithOnlyNameUrlAndDescription_thenExpectMatchingAccountManager()
            throws SAXException {
        accountManagerParser.startElement(null, RPCCommonTags.ACCOUNT_MANAGER, null, null);
        accountManagerParser.startElement(null, RPCCommonTags.NAME, null, null);
        accountManagerParser.characters(ACCOUNT_MANAGER.toCharArray(), 0, ACCOUNT_MANAGER.length());
        accountManagerParser.endElement(null, RPCCommonTags.NAME, null);
        accountManagerParser.startElement(null, RPCCommonTags.URL, null, null);
        accountManagerParser.characters("URL".toCharArray(), 0, 3);
        accountManagerParser.endElement(null, RPCCommonTags.URL, null);
        accountManagerParser.startElement(null, RPCCommonTags.DESCRIPTION, null, null);
        accountManagerParser.characters(DESCRIPTION.toCharArray(), 0, DESCRIPTION.length());
        accountManagerParser.endElement(null, RPCCommonTags.DESCRIPTION, null);
        accountManagerParser.endElement(null, RPCCommonTags.ACCOUNT_MANAGER, null);

        expected.setName(ACCOUNT_MANAGER);
        expected.setUrl("URL");
        expected.setDescription(DESCRIPTION);

        assertEquals(1, accountManagers.size());
        assertEquals(expected, accountManagers.get(0));
    }

    @Test
    public void testParser_whenXmlAccountManagerHasAllAttributes_thenExpectMatchingAccountManager()
            throws SAXException {
        accountManagerParser.startElement(null, RPCCommonTags.ACCOUNT_MANAGER, null, null);
        accountManagerParser.startElement(null, RPCCommonTags.NAME, null, null);
        accountManagerParser.characters(ACCOUNT_MANAGER.toCharArray(), 0, ACCOUNT_MANAGER.length());
        accountManagerParser.endElement(null, RPCCommonTags.NAME, null);
        accountManagerParser.startElement(null, RPCCommonTags.URL, null, null);
        accountManagerParser.characters("URL".toCharArray(), 0, 3);
        accountManagerParser.endElement(null, RPCCommonTags.URL, null);
        accountManagerParser.startElement(null, RPCCommonTags.DESCRIPTION, null, null);
        accountManagerParser.characters(DESCRIPTION.toCharArray(), 0, DESCRIPTION.length());
        accountManagerParser.endElement(null, RPCCommonTags.DESCRIPTION, null);
        accountManagerParser.startElement(null, AccountManagerParser.IMAGE_TAG, null, null);
        accountManagerParser.characters(IMAGE_URL.toCharArray(), 0, IMAGE_URL.length());
        accountManagerParser.endElement(null, AccountManagerParser.IMAGE_TAG, null);
        accountManagerParser.endElement(null, RPCCommonTags.ACCOUNT_MANAGER, null);

        expected.setName(ACCOUNT_MANAGER);
        expected.setUrl("URL");
        expected.setDescription(DESCRIPTION);
        expected.setImageUrl(IMAGE_URL);

        assertEquals(1, accountManagers.size());
        assertEquals(expected, accountManagers.get(0));
    }

    @Test
    public void testParser_whenTwoXmlAccountManagersHaveAllAttributes_thenExpectTwoMatchingAccountManagers()
            throws SAXException {
        accountManagerParser.startElement(null, RPCCommonTags.ACCOUNT_MANAGER, null, null);
        accountManagerParser.startElement(null, RPCCommonTags.NAME, null, null);
        accountManagerParser.characters((ACCOUNT_MANAGER + " 1").toCharArray(), 0,
                                        ACCOUNT_MANAGER.length() + 2);
        accountManagerParser.endElement(null, RPCCommonTags.NAME, null);
        accountManagerParser.startElement(null, RPCCommonTags.URL, null, null);
        accountManagerParser.characters("URL 1".toCharArray(), 0, 5);
        accountManagerParser.endElement(null, RPCCommonTags.URL, null);
        accountManagerParser.startElement(null, RPCCommonTags.DESCRIPTION, null, null);
        accountManagerParser.characters((DESCRIPTION + " 1").toCharArray(), 0,
                                        DESCRIPTION.length() + 2);
        accountManagerParser.endElement(null, RPCCommonTags.DESCRIPTION, null);
        accountManagerParser.startElement(null, AccountManagerParser.IMAGE_TAG, null, null);
        accountManagerParser.characters((IMAGE_URL + " 1").toCharArray(), 0, IMAGE_URL.length() + 2);
        accountManagerParser.endElement(null, AccountManagerParser.IMAGE_TAG, null);
        accountManagerParser.endElement(null, RPCCommonTags.ACCOUNT_MANAGER, null);

        accountManagerParser.startElement(null, RPCCommonTags.ACCOUNT_MANAGER, null, null);
        accountManagerParser.startElement(null, RPCCommonTags.NAME, null, null);
        accountManagerParser.characters((ACCOUNT_MANAGER + " 2").toCharArray(), 0, 17);
        accountManagerParser.endElement(null, RPCCommonTags.NAME, null);
        accountManagerParser.startElement(null, RPCCommonTags.URL, null, null);
        accountManagerParser.characters("URL 2".toCharArray(), 0, 5);
        accountManagerParser.endElement(null, RPCCommonTags.URL, null);
        accountManagerParser.startElement(null, RPCCommonTags.DESCRIPTION, null, null);
        accountManagerParser.characters((DESCRIPTION + " 2").toCharArray(), 0,
                                        DESCRIPTION.length() + 2);
        accountManagerParser.endElement(null, RPCCommonTags.DESCRIPTION, null);
        accountManagerParser.startElement(null, AccountManagerParser.IMAGE_TAG, null, null);
        accountManagerParser.characters((IMAGE_URL + " 2").toCharArray(), 0, IMAGE_URL.length() + 2);
        accountManagerParser.endElement(null, AccountManagerParser.IMAGE_TAG, null);
        accountManagerParser.endElement(null, RPCCommonTags.ACCOUNT_MANAGER, null);

        expected.setName(ACCOUNT_MANAGER + " 1");
        expected.setUrl("URL 1");
        expected.setDescription(DESCRIPTION + " 1");
        expected.setImageUrl(IMAGE_URL + " 1");
        final AccountManager expected2 = new AccountManager(ACCOUNT_MANAGER + " 2",
                                                            "URL 2",
                                                            DESCRIPTION + " 2",
                                                            IMAGE_URL + " 2");

        assertEquals(2, accountManagers.size());
        assertEquals(expected, accountManagers.get(0));
        assertEquals(expected2, accountManagers.get(1));
    }
}
