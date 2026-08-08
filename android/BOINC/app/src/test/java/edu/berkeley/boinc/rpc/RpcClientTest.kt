/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2022 University of California
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

import android.util.Log
import android.util.Xml
import edu.berkeley.boinc.rpc.RpcClient.Auth1Parser
import edu.berkeley.boinc.rpc.RpcClient.Auth2Parser
import io.mockk.*
import io.mockk.impl.annotations.MockK
import io.mockk.impl.annotations.SpyK
import io.mockk.junit5.MockKExtension
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.jupiter.api.extension.ExtendWith
import org.xml.sax.SAXException
import java.io.IOException

@ExtendWith(MockKExtension::class)
class RpcClientTest {
    @MockK
    private lateinit var simpleReplyParser: SimpleReplyParser

    @SpyK
    private var rpcClient = RpcClient()
    private lateinit var auth1Parser: Auth1Parser
    private lateinit var auth2Parser: Auth2Parser

    @Before
    fun setUp() {
        MockKAnnotations.init(this, overrideRecordPrivateCalls = true)

        mockkStatic(Log::class)
        every { Log.e(any(), any()) } returns 0
        every { Log.e(any(), any(), any()) } returns 0

        auth1Parser = Auth1Parser(StringBuilder())
        auth2Parser = Auth2Parser(StringBuilder())
    }

    @Test(expected = NullPointerException::class)
    @Throws(SAXException::class)
    fun `Auth1Parser() When nonce is null then expect NullPointerException`() {
        auth1Parser.characters(null, 0, 0)
    }

    @Test
    @Throws(SAXException::class)
    fun `Auth1Parser() When nonce is not null then expect nonce`() {
        auth1Parser.startElement(null, "nonce", null, null)
        auth1Parser.characters("Nonce".toCharArray(), 0, 5)
        auth1Parser.endElement(null, "nonce", null)
        Assert.assertEquals("Nonce", auth1Parser.mResult.toString())
    }

    @Test
    @Throws(SAXException::class)
    fun `Auth2Parser() When tag is null then expect empty Result`() {
        auth2Parser.startElement(null, null, null, null)
        auth2Parser.endElement(null, null, null)
        Assert.assertEquals(0, auth2Parser.mResult.length.toLong())
    }

    @Test
    @Throws(SAXException::class)
    fun `Auth2Parser() When tag is authorized then expect Authorized`() {
        auth2Parser.startElement(null, RpcClient.AUTHORIZED, null, null)
        auth2Parser.endElement(null, RpcClient.AUTHORIZED, null)
        Assert.assertEquals(RpcClient.AUTHORIZED, auth2Parser.mResult.toString())
    }

    @Test
    @Throws(SAXException::class)
    fun `Auth2Parser() When tag is unauthorized then expect Unauthorized`() {
        auth2Parser.startElement(null, RpcClient.UNAUTHORIZED, null, null)
        auth2Parser.endElement(null, RpcClient.UNAUTHORIZED, null)
        Assert.assertEquals(RpcClient.UNAUTHORIZED, auth2Parser.mResult.toString())
    }

    @Test(expected = NullPointerException::class)
    fun `CreateAccount() When AccountIn is null then expect NullPointerException`() {
        rpcClient.createAccount(null)
    }

    @Test
    fun `CreateAccount() When AccountIn is default then expect matching Xml string and success to be true`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(SimpleReplyParser)
        every { SimpleReplyParser.parse(any()) } returns simpleReplyParser
        every { simpleReplyParser.result } answers { true }
        every { simpleReplyParser.errorMessage } answers { fieldValue }
        val expected = String.format(
            CREATE_ACCOUNT_TEMPLATE, null, null,
            "24d794dfc756320ffadb905d526299bc", "", ""
        )
        Assert.assertNotNull(simpleReplyParser)
        Assert.assertTrue(simpleReplyParser.result)
        Assert.assertTrue(rpcClient.createAccount(AccountIn()))
        Assert.assertEquals(expected, rpcClient.mRequest.toString())
    }

    @Test
    fun `CreateAccount() When AccountIn has values set then expect matching Xml string and success to be true`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(SimpleReplyParser)
        every { SimpleReplyParser.parse(any()) } returns simpleReplyParser
        every { simpleReplyParser.result } answers { true }
        every { simpleReplyParser.errorMessage } answers { fieldValue }
        val expected = String.format(
            CREATE_ACCOUNT_TEMPLATE, "URL", "Email",
            "1604225a81748244d19bf2e787d6ee6e", "User", "Team"
        )
        val accountIn = AccountIn()
        accountIn.url = "URL"
        accountIn.emailAddress = "Email"
        accountIn.password = "Password"
        accountIn.userName = "User"
        accountIn.teamName = "Team"
        Assert.assertTrue(rpcClient.createAccount(accountIn))
        Assert.assertEquals(expected, rpcClient.mRequest.toString())
    }

    @Test
    @Throws(IOException::class)
    fun `CreateAccount() When IOException is thrown then expect success to be false`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(SimpleReplyParser)
        every { SimpleReplyParser.parse(any()) } returns simpleReplyParser
        every { simpleReplyParser.result } answers { false }
        every { simpleReplyParser.errorMessage } answers { fieldValue }
        every { rpcClient.sendRequest(any()) } throws IOException()
        Assert.assertFalse(rpcClient.createAccount(AccountIn()))
    }

    @Test
    fun `CreateAccount() When SimpleReplyParser is null then expect success to be false`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(SimpleReplyParser)
        every { SimpleReplyParser.parse(any()) } returns null
        Assert.assertFalse(rpcClient.createAccount(AccountIn()))
    }

    @Test
    fun `CreateAccountPoll() expect default AccountOut`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(AccountOutParser)
        every { AccountOutParser.parse(any()) } returns AccountOut()
        Assert.assertEquals(AccountOut(), rpcClient.createAccountPoll())
    }

    @Test
    @Throws(IOException::class)
    fun `CreateAccountPoll() When IOException is thrown then expect null`() {
        mockkObject(AccountOutParser)
        every { rpcClient.sendRequest(any()) } throws IOException()
        Assert.assertNull(rpcClient.createAccountPoll())
    }

    @Test(expected = NullPointerException::class)
    fun `LookupAccount() When AccountIn is null then expect NullPointerException`() {
        rpcClient.lookupAccount(null)
    }

    @Test(expected = NullPointerException::class)
    fun `LookupAccount() When AccountIn is default then expect NullPointerException`() {
        rpcClient.lookupAccount(AccountIn())
    }

    @Test
    fun `LookupAccount() When AccountIn has values set then expect matching Xml string and success to be true`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(SimpleReplyParser)
        every { SimpleReplyParser.parse(any()) } returns simpleReplyParser
        every { simpleReplyParser.result } answers { true }
        every { simpleReplyParser.errorMessage } answers { fieldValue }
        val expected = """<lookup_account>
 <url>URL</url>
 <email_addr>user</email_addr>
 <passwd_hash>e2dacfb3a3149122e2c52740013b94e5</passwd_hash>
</lookup_account>
"""
        val accountIn = AccountIn()
        accountIn.url = "URL"
        accountIn.userName = "User"
        accountIn.usesName = true
        accountIn.password = "Password"
        Assert.assertTrue(rpcClient.lookupAccount(accountIn))
        Assert.assertEquals(expected, rpcClient.mRequest.toString())
    }

    @Test
    fun `LookupAccount() When AccountIn has values set and SimpleReplyParser is null then expect matching Xml string and success to be false`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(SimpleReplyParser)
        every { SimpleReplyParser.parse(any()) } returns null
        val expected = """<lookup_account>
 <url>URL</url>
 <email_addr>user</email_addr>
 <passwd_hash>e2dacfb3a3149122e2c52740013b94e5</passwd_hash>
</lookup_account>
"""
        val accountIn = AccountIn()
        accountIn.url = "URL"
        accountIn.userName = "User"
        accountIn.usesName = true
        accountIn.password = "Password"
        Assert.assertFalse(rpcClient.lookupAccount(accountIn))
        Assert.assertEquals(expected, rpcClient.mRequest.toString())
    }

    @Test
    @Throws(IOException::class)
    fun `LookupAccount() When IOException is thrown then expect success to be false`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(SimpleReplyParser)
        every { SimpleReplyParser.parse(any()) } returns simpleReplyParser
        every { simpleReplyParser.result } answers { false }
        every { simpleReplyParser.errorMessage } answers { fieldValue }
        every { rpcClient.sendRequest(any()) } throws IOException()
        val accountIn = AccountIn()
        accountIn.url = "URL"
        accountIn.userName = "User"
        accountIn.usesName = true
        accountIn.password = "Password"
        Assert.assertFalse(rpcClient.lookupAccount(accountIn))
    }

    @Test
    fun `LookupAccountPoll() expect default AccountOut`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(AccountOutParser)
        every { AccountOutParser.parse(any()) } returns AccountOut()
        Assert.assertEquals(AccountOut(), rpcClient.lookupAccountPoll())
    }

    @Test
    @Throws(IOException::class)
    fun `LookupAccountPoll() When IOException is thrown then expect null`() {
        mockkObject(AccountOutParser)
        every { rpcClient.sendRequest(any()) } throws IOException()
        Assert.assertNull(rpcClient.lookupAccountPoll())
    }

    @Test
    fun `AcctMgrRPCPoll() expect default AcctMgrRPCReply`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(AcctMgrRPCReplyParser)
        every { AcctMgrRPCReplyParser.parse(any()) } returns AcctMgrRPCReply()
        Assert.assertEquals(AcctMgrRPCReply(), rpcClient.acctMgrRPCPoll())
    }

    @Test
    @Throws(IOException::class)
    fun `AcctMgrRPCPoll() when IOException is thrown then expect null`() {
        mockkObject(AccountOutParser)
        every { rpcClient.sendRequest(any()) } throws IOException()
        Assert.assertNull(rpcClient.acctMgrRPCPoll())
    }

    @Test
    fun `GetGlobalPreferencesWorkingStruct() expect default GlobalPreferences`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(GlobalPreferencesParser)
        every { GlobalPreferencesParser.parse(any()) } returns GlobalPreferences()
        Assert.assertEquals(GlobalPreferences(), rpcClient.globalPrefsWorkingStruct)
    }

    @Test
    @Throws(IOException::class)
    fun `GetGlobalPreferencesWorkingStruct() When IOException is thrown then expect null`() {
        mockkObject(AccountOutParser)
        every { rpcClient.sendRequest(any()) } throws IOException()
        Assert.assertNull(rpcClient.globalPrefsWorkingStruct)
    }

    @Test(expected = NullPointerException::class)
    fun `SetGlobalPrefsOverrideStruct() When GlobalPrefs is null then expect NullPointerException`() {
        rpcClient.setGlobalPrefsOverrideStruct(null)
    }

    @Test
    fun `SetGlobalPrefsOverrideStruct() When GlobalPrefs is default then expect matching Xml string and success to be true`() {
        val expected = String.format(
            GLOBAL_PREFS_TEMPLATE, 0, 0.0, 0.0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0, 0, "\n"
        )
        Assert.assertTrue(rpcClient.setGlobalPrefsOverrideStruct(GlobalPreferences()))
        Assert.assertEquals(expected, rpcClient.mRequest.toString())
    }

    @Test
    fun `SetGlobalPrefsOverrideStruct() When GlobalPrefs have default cpu_times then expect matching Xml string and success to be true`() {
        val stringBuilder = StringBuilder()
        for (i in 0..6) {
            stringBuilder.append(String.format(WEEK_PREFS_TEMPLATE, i, 0.0, 0.0))
        }
        val expected = String.format(
            GLOBAL_PREFS_TEMPLATE, 0, 0.0, 0.0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0, 0, "$stringBuilder"
        ).replace("\\s+".toRegex(), "")
        val globalPreferences = GlobalPreferences()
        for (i in 0..6) {
            globalPreferences.cpuTimes.weekPrefs[i] = TimeSpan()
        }
        Assert.assertTrue(rpcClient.setGlobalPrefsOverrideStruct(globalPreferences))
        val actual = rpcClient.mRequest.toString().replace("\\s+".toRegex(), "")
        Assert.assertEquals(expected, actual)
    }

    @Test
    fun `SetGlobalPrefsOverrideStruct() When GlobalPrefs have default net_times then expect matching Xml string and success to be true`() {
        val stringBuilder = StringBuilder()
        for (i in 0..6) {
            stringBuilder.append(String.format(WEEK_NET_PREFS_TEMPLATE, i, 0.0, 0.0))
        }
        val expected = String.format(
            GLOBAL_PREFS_TEMPLATE, 0, 0.0, 0.0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0, 0, "$stringBuilder"
        ).replace("\\s+".toRegex(), "")
        val globalPreferences = GlobalPreferences()
        for (i in 0..6) {
            globalPreferences.netTimes.weekPrefs[i] = TimeSpan()
        }
        Assert.assertTrue(rpcClient.setGlobalPrefsOverrideStruct(globalPreferences))
        val actual = rpcClient.mRequest.toString().replace("\\s+".toRegex(), "")
        Assert.assertEquals(expected, actual)
    }

    @Test
    @Throws(IOException::class)
    fun `SetGlobalPrefsOverrideStruct() When IOException is thrown then expect success to be false`() {
        every { rpcClient.sendRequest(any()) } throws IOException()
        Assert.assertFalse(rpcClient.setGlobalPrefsOverrideStruct(GlobalPreferences()))
    }

    @Test
    fun `ReadGlobalPrefsOverride() expect success to be true`() {
        Assert.assertTrue(rpcClient.readGlobalPrefsOverride())
    }

    @Test
    @Throws(IOException::class)
    fun `ReadGlobalPrefsOverride() When IOException is thrown then expect success to be false`() {
        every { rpcClient.sendRequest(any()) } throws IOException()
        Assert.assertFalse(rpcClient.readGlobalPrefsOverride())
    }

    @Test
    fun `RunBenchmarks() expect success to be true`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(SimpleReplyParser)
        every { SimpleReplyParser.parse(any()) } returns simpleReplyParser
        every { simpleReplyParser.result } answers { true }
        every { simpleReplyParser.errorMessage } answers { fieldValue }
        Assert.assertTrue(rpcClient.runBenchmarks())
    }

    @Test
    fun `RunBenchmarks() When SimpleReplyParser is null then expect success to be false`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(SimpleReplyParser)
        every { SimpleReplyParser.parse(any()) } returns null
        Assert.assertFalse(rpcClient.runBenchmarks())
    }

    @Test
    @Throws(IOException::class)
    fun `RunBenchmarks() When IOException is thrown then expect success to be false`() {
        mockkStatic(Xml::class)
        justRun { Xml.parse(any<String>(), any()) }
        mockkObject(SimpleReplyParser)
        every { SimpleReplyParser.parse(any()) } returns simpleReplyParser
        every { simpleReplyParser.result } answers { true }
        every { simpleReplyParser.errorMessage } answers { fieldValue }
        every { rpcClient.sendRequest(any()) } throws IOException()
        Assert.assertFalse(rpcClient.runBenchmarks())
    }

    companion object {
        private const val CREATE_ACCOUNT_TEMPLATE = "<create_account>\n" +
                "   <url>%s</url>\n" +
                "   <email_addr>%s</email_addr>\n" +
                "   <passwd_hash>%s</passwd_hash>\n" +
                "   <user_name>%s</user_name>\n" +
                "   <team_name>%s</team_name>\n" +
                "</create_account>\n"
        private const val GLOBAL_PREFS_TEMPLATE =
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
                    "</global_preferences>\n</set_global_prefs_override>\n"
        private const val WEEK_PREFS_TEMPLATE = "\n  <day_prefs>\n" +
                "    <day_of_week>%d</day_of_week>\n" +
                "    <start_hour>%.1f</start_hour>\n" +
                "    <end_hour>%.1f</end_hour>\n" +
                "  </day_prefs>"
        private const val WEEK_NET_PREFS_TEMPLATE = "\n  <day_prefs>\n" +
                "    <day_of_week>%d</day_of_week>\n" +
                "    <net_start_hour>%.1f</net_start_hour>\n" +
                "    <net_end_hour>%.1f</net_end_hour>\n" +
                "  </day_prefs>"
    }
}
