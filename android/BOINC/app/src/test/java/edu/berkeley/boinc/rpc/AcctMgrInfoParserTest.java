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
public class AcctMgrInfoParserTest {
    private static final String ACCT_MGR_NAME = "Account Manager Name";
    private static final String ACCT_MGR_URL = "Account Manager URL";
    private static final String COOKIE_FAIL_URL = "Cookie Failure URL";

    private AcctMgrInfoParser acctMgrInfoParser;
    private AcctMgrInfo expected;

    @Before
    public void setUp() {
        acctMgrInfoParser = new AcctMgrInfoParser();
        expected = new AcctMgrInfo();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectNull() {
        mockStatic(Xml.class);

        assertNull(AcctMgrInfoParser.parse(null));
    }

    @Test
    public void testParse_whenRpcStringIsEmpty_thenExpectNull() {
        mockStatic(Xml.class);

        assertNull(AcctMgrInfoParser.parse(""));
    }

    @Test
    public void testParser_whenLocalNameIsNull_thenExpectIllegalArgumentExceptionAndNullAccountManagerInfo() {
        assertThrows(IllegalArgumentException.class, () ->
                acctMgrInfoParser.startElement(null, null, null, null));
        assertNull(acctMgrInfoParser.getAccountMgrInfo());
    }

    @Test
    public void testParser_whenOnlyStartElementIsRun_thenExpectElementStarted() throws SAXException {
        acctMgrInfoParser.startElement(null, "", null, null);

        assertTrue(acctMgrInfoParser.mElementStarted);
    }

    @Test
    public void testParser_whenBothStartElementAndEndElementAreRun_thenExpectElementNotStarted() throws SAXException {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null);
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null);

        assertFalse(acctMgrInfoParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectNullAccountManagerInfo() throws SAXException {
        acctMgrInfoParser.startElement(null, "", null, null);

        assertNull(acctMgrInfoParser.getAccountMgrInfo());
    }

    @Test
    public void testParser_whenXmlAccountManagerInfoHasNoElements_thenExpectDefaultAccountManagerInfo()
            throws SAXException {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null);
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null);

        assertEquals(expected, acctMgrInfoParser.getAccountMgrInfo());
    }

    @Test
    public void testParser_whenXmlAccountManagerInfoHasOnlyName_thenExpectMatchingAccountManagerInfo()
            throws SAXException {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null, null);
        acctMgrInfoParser.characters(ACCT_MGR_NAME.toCharArray(), 0, ACCT_MGR_NAME.length());
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null);
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null);

        expected.setAcctMgrName(ACCT_MGR_NAME);

        assertEquals(expected, acctMgrInfoParser.getAccountMgrInfo());
    }

    @Test
    public void testParser_whenXmlAccountManagerInfoHasOnlyNameAndUrl_thenExpectMatchingAccountManagerInfo()
            throws SAXException {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null, null);
        acctMgrInfoParser.characters(ACCT_MGR_NAME.toCharArray(), 0, ACCT_MGR_NAME.length());
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null, null);
        acctMgrInfoParser.characters(ACCT_MGR_URL.toCharArray(), 0, ACCT_MGR_URL.length());
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null);
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null);

        expected.setAcctMgrName(ACCT_MGR_NAME);
        expected.setAcctMgrUrl(ACCT_MGR_URL);

        assertEquals(expected, acctMgrInfoParser.getAccountMgrInfo());
    }

    @Test
    public void testParser_whenXmlAccountManagerInfoHasOnlyNameUrlAndCookieFailureUrl_thenExpectMatchingAccountManagerInfo()
            throws SAXException {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null, null);
        acctMgrInfoParser.characters(ACCT_MGR_NAME.toCharArray(), 0, ACCT_MGR_NAME.length());
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null, null);
        acctMgrInfoParser.characters(ACCT_MGR_URL.toCharArray(), 0, ACCT_MGR_URL.length());
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.COOKIE_FAILURE_URL, null, null);
        acctMgrInfoParser.characters(COOKIE_FAIL_URL.toCharArray(), 0, COOKIE_FAIL_URL.length());
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.COOKIE_FAILURE_URL, null);
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null);

        expected.setAcctMgrName(ACCT_MGR_NAME);
        expected.setAcctMgrUrl(ACCT_MGR_URL);
        expected.setCookieFailureUrl(COOKIE_FAIL_URL);

        assertEquals(expected, acctMgrInfoParser.getAccountMgrInfo());
    }

    @Test
    public void testParser_whenXmlAccountManagerInfoHasOnlyNameUrlCookieFailureUrlAndHaveCredentials_thenExpectMatchingAccountManagerInfo()
            throws SAXException {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null, null);
        acctMgrInfoParser.characters(ACCT_MGR_NAME.toCharArray(), 0, ACCT_MGR_NAME.length());
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null, null);
        acctMgrInfoParser.characters(ACCT_MGR_URL.toCharArray(), 0, ACCT_MGR_URL.length());
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.COOKIE_FAILURE_URL, null, null);
        acctMgrInfoParser.characters(COOKIE_FAIL_URL.toCharArray(), 0, COOKIE_FAIL_URL.length());
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.COOKIE_FAILURE_URL, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.HAVING_CREDENTIALS, null, null);
        acctMgrInfoParser.characters("true".toCharArray(), 0, 4);
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.HAVING_CREDENTIALS, null);
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null);

        expected = new AcctMgrInfo(ACCT_MGR_NAME, ACCT_MGR_URL, COOKIE_FAIL_URL,
                                   true, false, true);

        assertEquals(expected, acctMgrInfoParser.getAccountMgrInfo());
    }

    @Test
    public void testParser_whenXmlAccountManagerInfoHasOnlyNameUrlCookieFailureUrlHaveCredentialsAndCookieRequired_thenExpectMatchingAccountManagerInfo()
            throws SAXException {
        acctMgrInfoParser.startElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null, null);
        acctMgrInfoParser.characters(ACCT_MGR_NAME.toCharArray(), 0, ACCT_MGR_NAME.length());
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_NAME, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null, null);
        acctMgrInfoParser.characters(ACCT_MGR_URL.toCharArray(), 0, ACCT_MGR_URL.length());
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.ACCT_MGR_URL, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.COOKIE_FAILURE_URL, null, null);
        acctMgrInfoParser.characters(COOKIE_FAIL_URL.toCharArray(), 0, COOKIE_FAIL_URL.length());
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.COOKIE_FAILURE_URL, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.HAVING_CREDENTIALS, null, null);
        acctMgrInfoParser.characters("true".toCharArray(), 0, 4);
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.HAVING_CREDENTIALS, null);
        acctMgrInfoParser.startElement(null, AcctMgrInfo.Fields.COOKIE_REQUIRED, null, null);
        acctMgrInfoParser.characters("true".toCharArray(), 0, 4);
        acctMgrInfoParser.endElement(null, AcctMgrInfo.Fields.COOKIE_REQUIRED, null);
        acctMgrInfoParser.endElement(null, AcctMgrInfoParser.ACCT_MGR_INFO_TAG, null);

        expected = new AcctMgrInfo(ACCT_MGR_NAME, ACCT_MGR_URL, COOKIE_FAIL_URL,
                                   true, true, true);

        assertEquals(expected, acctMgrInfoParser.getAccountMgrInfo());
    }
}