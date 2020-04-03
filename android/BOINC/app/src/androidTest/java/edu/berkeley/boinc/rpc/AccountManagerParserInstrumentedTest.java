package edu.berkeley.boinc.rpc;

import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
public class AccountManagerParserInstrumentedTest {
    @Test(expected = IllegalArgumentException.class)
    public void testParse_whenRpcStringIsNull_thenExpectIllegalArgumentException() {
        AccountManagerParser.parse(null);
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectEmptyList() {
        assertTrue(AccountManagerParser.parse("").isEmpty());
    }

    @Test
    public void testParse_whenXmlAccountManagerIsValid_thenExpectMatchingListWithAccountManager() {
        final String accountManager = "<account_manager>" +
                                      "<name>Account Manager</name>" +
                                      "<url>https://account-manager.org</url>" +
                                      "<description>This is an account manager.</description>" +
                                      "<image>https://account-manager.org/image.jpg</image>" +
                                      "</account_manager>";
        final List<AccountManager> accountManagers = AccountManagerParser.parse(accountManager);

        assertFalse(accountManagers.isEmpty());
        assertEquals(new AccountManager("Account Manager",
                                        "https://account-manager.org",
                                        "This is an account manager.",
                                        "https://account-manager.org/image.jpg"),
                     accountManagers.get(0));
    }

    @Test
    public void testParse_whenXmlAccountManagerIsInvalid_thenExpectEmptyList() {
        final String accountManager = "<account_manager>" +
                                      "<name>Account Manager</name>" +
                                      "<url>https://account-manager.org</url>" +
                                      "<description>This is an account manager.</description>" +
                                      "<image>https://account-manager.org/image.jpg</image>";

        assertTrue(AccountManagerParser.parse(accountManager).isEmpty());
    }
}
