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
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.xml.sax.SAXException;

import java.io.IOException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest({AccountOutParser.class, AcctMgrRPCReplyParser.class, GlobalPreferencesParser.class,
                 SimpleReplyParser.class, Md5.class})
public class RpcClientTest {
    private static final String CREATE_ACCOUNT_TEMPLATE = "<create_account>\n" +
                                                          "   <url>%s</url>\n" +
                                                          "   <email_addr>%s</email_addr>\n" +
                                                          "   <passwd_hash>%s</passwd_hash>\n" +
                                                          "   <user_name>%s</user_name>\n" +
                                                          "   <team_name>%s</team_name>\n" +
                                                          "</create_account>\n";
    private static final String GLOBAL_PREFS_TEMPLATE =
            "<set_global_prefs_override>\n<global_preferences>\n" +
            "  <run_on_batteries>%d</run_on_batteries>\n" +
            "  <battery_charge_min_pct>%.1f</battery_charge_min_pct>\n" +
            "  <battery_max_temperature>%.1f</battery_max_temperature>\n" +
            "  <run_gpu_if_user_active>%d</run_gpu_if_user_active>\n" +
            "  <run_if_user_active>%d</run_if_user_active>\n" +
            "  <idle_time_to_run>%.1f</idle_time_to_run>\n" +
            "  <suspend_cpu_usage>%.1f</suspend_cpu_usage>\n" +
            "  <start_hour>%.1f</start_hour>\n" +
            "  <end_hour>%.1f</end_hour>\n" +
            "  <net_start_hour>%.1f</net_start_hour>\n" +
            "  <net_end_hour>%.1f</net_end_hour>\n" +
            "  <max_ncpus_pct>%.1f</max_ncpus_pct>\n" +
            "  <leave_apps_in_memory>%d</leave_apps_in_memory>\n" +
            "  <dont_verify_images>%d</dont_verify_images>\n" +
            "  <work_buf_min_days>%.1f</work_buf_min_days>\n" +
            "  <work_buf_additional_days>%.1f</work_buf_additional_days>\n" +
            "  <disk_interval>%.1f</disk_interval>\n" +
            "  <cpu_scheduling_period_minutes>%.1f</cpu_scheduling_period_minutes>\n" +
            "  <disk_max_used_gb>%.1f</disk_max_used_gb>\n" +
            "  <disk_max_used_pct>%.1f</disk_max_used_pct>\n" +
            "  <disk_min_free_gb>%.1f</disk_min_free_gb>\n" +
            "  <ram_max_used_busy_pct>%.1f</ram_max_used_busy_pct>\n" +
            "  <ram_max_used_idle_pct>%.1f</ram_max_used_idle_pct>\n" +
            "  <max_bytes_sec_up>%.1f</max_bytes_sec_up>\n" +
            "  <max_bytes_sec_down>%.1f</max_bytes_sec_down>\n" +
            "  <cpu_usage_limit>%.1f</cpu_usage_limit>\n" +
            "  <daily_xfer_limit_mb>%.1f</daily_xfer_limit_mb>\n" +
            "  <daily_xfer_period_days>%d</daily_xfer_period_days>\n" +
            "  <network_wifi_only>%d</network_wifi_only>%s" +
            "</global_preferences>\n</set_global_prefs_override>\n";
    private static final String WEEK_PREFS_TEMPLATE = "\n  <day_prefs>\n" +
                                                      "    <day_of_week>%d</day_of_week>\n" +
                                                      "    <start_hour>%.1f</start_hour>\n" +
                                                      "    <end_hour>%.1f</end_hour>\n" +
                                                      "  </day_prefs>";
    private static final String WEEK_NET_PREFS_TEMPLATE = "\n  <day_prefs>\n" +
                                                          "    <day_of_week>%d</day_of_week>\n" +
                                                          "    <net_start_hour>%.1f</net_start_hour>\n" +
                                                          "    <net_end_hour>%.1f</net_end_hour>\n" +
                                                          "  </day_prefs>";

    @Mock
    private SimpleReplyParser simpleReplyParser;
    @Mock
    private RpcClient mockRpcClient;

    private RpcClient rpcClient;
    private RpcClient.Auth1Parser auth1Parser;
    private RpcClient.Auth2Parser auth2Parser;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(RpcClientTest.class);

        mockRpcClient.mRequest = new StringBuilder();
        mockRpcClient.mResult = new StringBuilder();

        rpcClient = new RpcClient();
        auth1Parser = new RpcClient.Auth1Parser(new StringBuilder());
        auth2Parser = new RpcClient.Auth2Parser(new StringBuilder());
    }

    @Test(expected = NullPointerException.class)
    public void testAuth1Parser_whenNonceIsNull_thenExpectNullPointerException() throws SAXException {
        auth1Parser.characters(null, 0, 0);
    }

    @Test
    public void testAuth1Parser_whenNonceIsNotNull_thenExpectNonce() throws SAXException {
        auth1Parser.startElement(null, "nonce", null, null);
        auth1Parser.characters("Nonce".toCharArray(), 0, 5);
        auth1Parser.endElement(null, "nonce", null);

        assertEquals("Nonce", auth1Parser.mResult.toString());
    }

    @Test
    public void testAuth2Parser_whenTagIsNull_thenExpectEmptyResult() throws SAXException {
        auth2Parser.startElement(null, null, null, null);
        auth2Parser.endElement(null, null, null);

        assertEquals(0, auth2Parser.mResult.length());
    }

    @Test
    public void testAuth2Parser_whenTagIsAuthorized_thenExpectAuthorized() throws SAXException {
        auth2Parser.startElement(null, RpcClient.AUTHORIZED, null, null);
        auth2Parser.endElement(null, RpcClient.AUTHORIZED, null);

        assertEquals(RpcClient.AUTHORIZED, auth2Parser.mResult.toString());
    }

    @Test
    public void testAuth2Parser_whenTagIsUnauthorized_thenExpectUnauthorized() throws SAXException {
        auth2Parser.startElement(null, RpcClient.UNAUTHORIZED, null, null);
        auth2Parser.endElement(null, RpcClient.UNAUTHORIZED, null);

        assertEquals(RpcClient.UNAUTHORIZED, auth2Parser.mResult.toString());
    }

    @Test(expected = NullPointerException.class)
    public void testCreateAccount_whenAccountInIsNull_thenExpectNullPointerException() {
        rpcClient.createAccount(null);
    }

    @Test
    public void testCreateAccount_whenAccountInIsDefault_thenExpectMatchingXmlStringAndSuccessToBeTrue() {
        mockStatic(Md5.class);
        mockStatic(SimpleReplyParser.class);

        PowerMockito.when(Md5.hash(any())).thenCallRealMethod();
        PowerMockito.when(SimpleReplyParser.parse(anyString())).thenReturn(simpleReplyParser);
        Mockito.when(simpleReplyParser.getResult()).thenReturn(true);

        final String expected = String.format(CREATE_ACCOUNT_TEMPLATE, null, null,
                                              "24d794dfc756320ffadb905d526299bc", "", "");

        assertTrue(rpcClient.createAccount(new AccountIn()));
        assertEquals(expected, rpcClient.mRequest.toString());
    }

    @Test
    public void testCreateAccount_whenAccountInHasValuesSet_thenExpectMatchingXmlStringAndSuccessToBeTrue() {
        mockStatic(Md5.class);
        mockStatic(SimpleReplyParser.class);

        PowerMockito.when(Md5.hash(any())).thenCallRealMethod();
        PowerMockito.when(SimpleReplyParser.parse(anyString())).thenReturn(simpleReplyParser);
        Mockito.when(simpleReplyParser.getResult()).thenReturn(true);

        final String expected = String.format(CREATE_ACCOUNT_TEMPLATE, "URL", "Email",
                                              "1604225a81748244d19bf2e787d6ee6e", "User", "Team");

        final AccountIn accountIn = new AccountIn();
        accountIn.setUrl("URL");
        accountIn.setEmailAddress("Email");
        accountIn.setPassword("Password");
        accountIn.setUserName("User");
        accountIn.setTeamName("Team");

        assertTrue(rpcClient.createAccount(accountIn));
        assertEquals(expected, rpcClient.mRequest.toString());
    }

    @Test
    public void testCreateAccount_whenIOExceptionIsThrown_thenExpectSuccessToBeFalse() throws IOException {
        mockStatic(Md5.class);
        mockStatic(SimpleReplyParser.class);

        PowerMockito.when(Md5.hash(any())).thenCallRealMethod();
        PowerMockito.when(SimpleReplyParser.parse(anyString())).thenReturn(simpleReplyParser);
        Mockito.doThrow(new IOException()).when(mockRpcClient).sendRequest(anyString());
        Mockito.when(simpleReplyParser.getResult()).thenReturn(false);
        Mockito.when(mockRpcClient.createAccount(any())).thenCallRealMethod();

        assertFalse(mockRpcClient.createAccount(new AccountIn()));
    }

    @Test
    public void testCreateAccount_whenSimpleReplyParserIsNull_thenExpectSuccessToBeFalse() {
        mockStatic(Md5.class);
        mockStatic(SimpleReplyParser.class);

        PowerMockito.when(Md5.hash(any())).thenCallRealMethod();
        PowerMockito.when(SimpleReplyParser.parse(anyString())).thenReturn(null);
        Mockito.when(mockRpcClient.createAccount(any())).thenCallRealMethod();

        assertFalse(mockRpcClient.createAccount(new AccountIn()));
    }

    @Test
    public void testCreateAccountPoll_thenExpectDefaultAccountOut() {
        mockStatic(AccountOutParser.class);

        PowerMockito.when(AccountOutParser.parse(anyString())).thenReturn(new AccountOut());

        assertEquals(new AccountOut(), rpcClient.createAccountPoll());
    }

    @Test
    public void testCreateAccountPoll_whenIOExceptionIsThrown_thenExpectNull() throws IOException {
        mockStatic(AccountOutParser.class);

        Mockito.doThrow(new IOException()).when(mockRpcClient).sendRequest(anyString());
        Mockito.when(mockRpcClient.createAccountPoll()).thenCallRealMethod();

        assertNull(mockRpcClient.createAccountPoll());
    }

    @Test(expected = NullPointerException.class)
    public void testLookupAccount_whenAccountInIsNull_thenExpectNullPointerException() {
        rpcClient.lookupAccount(null);
    }

    @Test(expected = NullPointerException.class)
    public void testLookupAccount_whenAccountInIsDefault_thenExpectNullPointerException() {
        rpcClient.lookupAccount(new AccountIn());
    }

    @Test
    public void testLookupAccount_whenAccountInHasValuesSet_thenExpectMatchingXmlStringAndSuccessToBeTrue() {
        mockStatic(Md5.class);
        mockStatic(SimpleReplyParser.class);

        PowerMockito.when(Md5.hash(any())).thenCallRealMethod();
        PowerMockito.when(SimpleReplyParser.parse(anyString())).thenReturn(simpleReplyParser);
        Mockito.when(simpleReplyParser.getResult()).thenReturn(true);

        final String expected = "<lookup_account>\n <url>URL</url>\n" +
                                " <email_addr>user</email_addr>\n" +
                                " <passwd_hash>e2dacfb3a3149122e2c52740013b94e5</passwd_hash>\n" +
                                "</lookup_account>\n";

        final AccountIn accountIn = new AccountIn();
        accountIn.setUrl("URL");
        accountIn.setUserName("User");
        accountIn.setUsesName(true);
        accountIn.setPassword("Password");

        assertTrue(rpcClient.lookupAccount(accountIn));
        assertEquals(expected, rpcClient.mRequest.toString());
    }

    @Test
    public void testLookupAccount_whenAccountInHasValuesSetAndSimpleReplyParserIsNull_thenExpectMatchingXmlStringAndSuccessToBeFalse() {
        mockStatic(Md5.class);
        mockStatic(SimpleReplyParser.class);

        PowerMockito.when(Md5.hash(any())).thenCallRealMethod();
        PowerMockito.when(SimpleReplyParser.parse(anyString())).thenReturn(null);

        final String expected = "<lookup_account>\n <url>URL</url>\n" +
                                " <email_addr>user</email_addr>\n" +
                                " <passwd_hash>e2dacfb3a3149122e2c52740013b94e5</passwd_hash>\n" +
                                "</lookup_account>\n";

        final AccountIn accountIn = new AccountIn();
        accountIn.setUrl("URL");
        accountIn.setUserName("User");
        accountIn.setUsesName(true);
        accountIn.setPassword("Password");

        assertFalse(rpcClient.lookupAccount(accountIn));
        assertEquals(expected, rpcClient.mRequest.toString());
    }

    @Test
    public void testLookupAccount_whenIOExceptionIsThrown_thenExpectSuccessToBeFalse()
            throws IOException {
        mockStatic(Md5.class);
        mockStatic(SimpleReplyParser.class);

        PowerMockito.when(Md5.hash(any())).thenCallRealMethod();
        PowerMockito.when(SimpleReplyParser.parse(anyString())).thenReturn(simpleReplyParser);
        Mockito.doThrow(new IOException()).when(mockRpcClient).sendRequest(anyString());
        Mockito.when(simpleReplyParser.getResult()).thenReturn(false);
        Mockito.when(mockRpcClient.lookupAccount(any())).thenCallRealMethod();

        final AccountIn accountIn = new AccountIn();
        accountIn.setUrl("URL");
        accountIn.setUserName("User");
        accountIn.setUsesName(true);
        accountIn.setPassword("Password");

        assertFalse(mockRpcClient.lookupAccount(accountIn));
    }

    @Test
    public void testLookupAccountPoll_thenExpectDefaultAccountOut() {
        mockStatic(AccountOutParser.class);

        PowerMockito.when(AccountOutParser.parse(anyString())).thenReturn(new AccountOut());

        assertEquals(new AccountOut(), rpcClient.lookupAccountPoll());
    }

    @Test
    public void testLookupAccountPoll_whenIOExceptionIsThrown_thenExpectNull() throws IOException {
        mockStatic(AccountOutParser.class);

        Mockito.doThrow(new IOException()).when(mockRpcClient).sendRequest(anyString());
        Mockito.when(mockRpcClient.lookupAccountPoll()).thenCallRealMethod();

        assertNull(mockRpcClient.lookupAccountPoll());
    }

    @Test
    public void testAcctMgrRPCPoll_expectDefaultAcctMgrRPCReply() {
        mockStatic(AcctMgrRPCReplyParser.class);

        PowerMockito.when(AcctMgrRPCReplyParser.parse(anyString())).thenReturn(new AcctMgrRPCReply());

        assertEquals(new AcctMgrRPCReply(), rpcClient.acctMgrRPCPoll());
    }

    @Test
    public void testAcctMgrRPCPoll_whenIOExceptionIsThrown_thenExpectNull() throws IOException {
        mockStatic(AccountOutParser.class);

        Mockito.doThrow(new IOException()).when(mockRpcClient).sendRequest(anyString());
        Mockito.when(mockRpcClient.acctMgrRPCPoll()).thenCallRealMethod();

        assertNull(mockRpcClient.acctMgrRPCPoll());
    }

    @Test
    public void testGetGlobalPreferencesWorkingStruct_expectDefaultGlobalPreferences() {
        mockStatic(GlobalPreferencesParser.class);

        PowerMockito.when(GlobalPreferencesParser.parse(anyString())).thenReturn(new GlobalPreferences());

        assertEquals(new GlobalPreferences(), rpcClient.getGlobalPrefsWorkingStruct());
    }

    @Test
    public void tesGetGlobalPreferencesWorkingStruct_whenIOExceptionIsThrown_thenExpectNull()
            throws IOException {
        mockStatic(AccountOutParser.class);

        Mockito.doThrow(new IOException()).when(mockRpcClient).sendRequest(anyString());
        Mockito.when(mockRpcClient.getGlobalPrefsWorkingStruct()).thenCallRealMethod();

        assertNull(mockRpcClient.getGlobalPrefsWorkingStruct());
    }

    @Test(expected = NullPointerException.class)
    public void testSetGlobalPrefsOverrideStruct_whenGlobalPrefsIsNull_thenExpectNullPointerException() {
        rpcClient.setGlobalPrefsOverrideStruct(null);
    }

    @Test
    public void testSetGlobalPrefsOverrideStruct_whenGlobalPrefsIsDefault_thenExpectMatchingXmlStringAndSuccessToBeTrue() {
        final String expected =
                String.format(GLOBAL_PREFS_TEMPLATE, 0, 0.0, 0.0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                              0.0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                              0.0, 0.0, 0.0, 0, 0, "\n");

        assertTrue(rpcClient.setGlobalPrefsOverrideStruct(new GlobalPreferences()));
        assertEquals(expected, rpcClient.mRequest.toString());
    }

    @Test
    public void testSetGlobalPrefsOverrideStruct_whenGlobalPrefsHaveDefaultCpuTimes_thenExpectMatchingXmlStringAndSuccessToBeTrue() {
        final StringBuilder stringBuilder = new StringBuilder();
        for(int i = 0; i < 7; i++) {
            stringBuilder.append(String.format(WEEK_PREFS_TEMPLATE, i, 0.0, 0.0));
        }
        final String expected =
                String.format(GLOBAL_PREFS_TEMPLATE, 0, 0.0, 0.0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                              0.0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                              0.0, 0.0, 0.0, 0, 0, stringBuilder.toString() + "\n");

        final GlobalPreferences globalPreferences = new GlobalPreferences();
        for(int i = 0; i < 7; i++) {
            globalPreferences.getCpuTimes().getWeekPrefs()[i] = new TimeSpan();
        }

        assertTrue(rpcClient.setGlobalPrefsOverrideStruct(globalPreferences));
        assertEquals(expected, rpcClient.mRequest.toString());
    }

    @Test
    public void testSetGlobalPrefsOverrideStruct_whenGlobalPrefsHaveDefaultNetTimes_thenExpectMatchingXmlStringAndSuccessToBeTrue() {
        final StringBuilder stringBuilder = new StringBuilder();
        for(int i = 0; i < 7; i++) {
            stringBuilder.append(String.format(WEEK_NET_PREFS_TEMPLATE, i, 0.0, 0.0));
        }
        final String expected =
                String.format(GLOBAL_PREFS_TEMPLATE, 0, 0.0, 0.0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                              0.0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                              0.0, 0.0, 0.0, 0, 0, stringBuilder.toString() + "\n");

        final GlobalPreferences globalPreferences = new GlobalPreferences();
        for(int i = 0; i < 7; i++) {
            globalPreferences.getNetTimes().getWeekPrefs()[i] = new TimeSpan();
        }

        assertTrue(rpcClient.setGlobalPrefsOverrideStruct(globalPreferences));
        assertEquals(expected, rpcClient.mRequest.toString());
    }

    @Test
    public void testSetGlobalPrefsOverrideStruct_whenIOExceptionIsThrown_thenExpectSuccessToBeFalse()
            throws IOException {
        Mockito.doThrow(new IOException()).when(mockRpcClient).sendRequest(anyString());
        Mockito.when(mockRpcClient.setGlobalPrefsOverrideStruct(any())).thenCallRealMethod();

        assertFalse(mockRpcClient.setGlobalPrefsOverrideStruct(new GlobalPreferences()));
    }

    @Test
    public void testReadGlobalPrefsOverride_expectSuccessToBeTrue() {
        assertTrue(rpcClient.readGlobalPrefsOverride());
    }

    @Test
    public void testReadGlobalPrefsOverride_whenIOExceptionIsThrown_thenExpectSuccessToBeFalse()
            throws IOException {
        Mockito.doThrow(new IOException()).when(mockRpcClient).sendRequest(anyString());
        Mockito.when(mockRpcClient.readGlobalPrefsOverride()).thenCallRealMethod();

        assertFalse(mockRpcClient.readGlobalPrefsOverride());
    }

    @Test
    public void testRunBenchmarks_expectSuccessToBeTrue() {
        mockStatic(SimpleReplyParser.class);

        PowerMockito.when(SimpleReplyParser.parse(anyString())).thenReturn(simpleReplyParser);
        Mockito.when(simpleReplyParser.getResult()).thenReturn(true);

        assertTrue(rpcClient.runBenchmarks());
    }

    @Test
    public void testRunBenchmarks_whenSimpleReplyParserIsNull_thenExpectSuccessToBeFalse() {
        mockStatic(SimpleReplyParser.class);

        PowerMockito.when(SimpleReplyParser.parse(anyString())).thenReturn(null);

        assertFalse(rpcClient.runBenchmarks());
    }

    @Test
    public void testRunBenchmarks_whenIOExceptionIsThrown_thenExpectSuccessToBeFalse()
            throws IOException {
        mockStatic(SimpleReplyParser.class);

        PowerMockito.when(SimpleReplyParser.parse(anyString())).thenReturn(simpleReplyParser);
        Mockito.when(simpleReplyParser.getResult()).thenReturn(true);
        Mockito.doThrow(new IOException()).when(mockRpcClient).sendRequest(anyString());
        Mockito.when(mockRpcClient.runBenchmarks()).thenCallRealMethod();

        assertFalse(mockRpcClient.runBenchmarks());
    }
}
