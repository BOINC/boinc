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

import android.util.Log
import android.util.Xml
import edu.berkeley.boinc.rpc.ProjectConfigReplyParser.Companion.parse
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.ArgumentMatchers
import org.powermock.api.mockito.PowerMockito
import org.powermock.core.classloader.annotations.PrepareForTest
import org.powermock.modules.junit4.PowerMockRunner
import org.xml.sax.ContentHandler
import org.xml.sax.SAXException

@RunWith(PowerMockRunner::class)
@PrepareForTest(Log::class, Xml::class)
class ProjectConfigReplyParserTest {
    private lateinit var projectConfigReplyParser: ProjectConfigReplyParser
    private lateinit var expected: ProjectConfig
    @Before
    fun setUp() {
        projectConfigReplyParser = ProjectConfigReplyParser()
        expected = ProjectConfig()
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    fun `When Rpc string is empty then expect UninitializedPropertyAccessException`() {
        PowerMockito.mockStatic(Xml::class.java)
        parse("")
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown then expect null`() {
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertNull(parse(""))
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect element started`() {
        projectConfigReplyParser.startElement(null, "", null, null)
        Assert.assertTrue(projectConfigReplyParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is project_config tag then expect element not started`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        Assert.assertFalse(projectConfigReplyParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has name then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(null, NAME, null, null)
        projectConfigReplyParser.characters(
            "Project Config".toCharArray(), 0,
            "Project Config".length
        )
        projectConfigReplyParser.endElement(null, NAME, null)
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.name = "Project Config"
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has master_url then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, MASTER_URL,
            null, null
        )
        projectConfigReplyParser.characters("Master URL".toCharArray(), 0, "Master URL".length)
        projectConfigReplyParser.endElement(
            null, MASTER_URL,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.masterUrl = "Master URL"
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has web_rpc_url_base then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.WEB_RPC_URL_BASE,
            null, null
        )
        projectConfigReplyParser.characters(
            "Web RPC URL Base".toCharArray(), 0,
            "Web RPC URL Base".length
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.WEB_RPC_URL_BASE,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.webRpcUrlBase = "Web RPC URL Base"
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has local_revision then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.LOCAL_REVISION,
            null, null
        )
        projectConfigReplyParser.characters(
            "Local Revision".toCharArray(), 0,
            "Local Revision".length
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.LOCAL_REVISION,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.localRevision = "Local Revision"
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has invalid min_pwd_length then expect default ProjectConfig`() {
        PowerMockito.mockStatic(Log::class.java)
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.MIN_PWD_LENGTH,
            null, null
        )
        projectConfigReplyParser.characters("Eight".toCharArray(), 0, 5)
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.MIN_PWD_LENGTH,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has min_pwd_length then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.MIN_PWD_LENGTH,
            null, null
        )
        projectConfigReplyParser.characters("8".toCharArray(), 0, 1)
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.MIN_PWD_LENGTH,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.minPwdLength = 8
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has user_name tag then expect ProjectConfig with user_name true`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.USER_NAME_TAG,
            null, null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.USER_NAME_TAG,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.usesName = true
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has uses_user_name tag then expect ProjectConfig with uses_user_name true`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.USES_USER_NAME_TAG,
            null, null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.USES_USER_NAME_TAG,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.usesName = true
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has web_stopped 0 then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.WEB_STOPPED,
            null, null
        )
        projectConfigReplyParser.characters("0".toCharArray(), 0, 1)
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.WEB_STOPPED,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.webStopped = false
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has web_stopped 1 then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.WEB_STOPPED,
            null, null
        )
        projectConfigReplyParser.characters("1".toCharArray(), 0, 1)
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.WEB_STOPPED,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.webStopped = true
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has scheduler_stopped 0 then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.SCHEDULER_STOPPED,
            null, null
        )
        projectConfigReplyParser.characters("0".toCharArray(), 0, 1)
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.SCHEDULER_STOPPED,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.schedulerStopped = false
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has scheduler stopped 1 then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.SCHEDULER_STOPPED,
            null, null
        )
        projectConfigReplyParser.characters("1".toCharArray(), 0, 1)
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.SCHEDULER_STOPPED,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.schedulerStopped = true
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has client_account_creation_disabled tag then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.CLIENT_ACCOUNT_CREATION_DISABLED,
            null, null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.CLIENT_ACCOUNT_CREATION_DISABLED,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.clientAccountCreationDisabled = true
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has min_client_version then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.MIN_CLIENT_VERSION,
            null, null
        )
        projectConfigReplyParser.characters("1".toCharArray(), 0, 1)
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.MIN_CLIENT_VERSION,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.minClientVersion = 1
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has rpc_prefix then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.RPC_PREFIX,
            null, null
        )
        projectConfigReplyParser.characters("RPC Prefix".toCharArray(), 0, "RPC Prefix".length)
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.RPC_PREFIX,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.rpcPrefix = "RPC Prefix"
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has error_num then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(null, ERROR_NUM, null, null)
        projectConfigReplyParser.characters("1".toCharArray(), 0, 1)
        projectConfigReplyParser.endElement(null, ERROR_NUM, null)
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.errorNum = 1
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has account_manager tag then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.ACCOUNT_MANAGER,
            null, null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.ACCOUNT_MANAGER,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.accountManager = true
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has terms_of_use then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.TERMS_OF_USE,
            null, null
        )
        projectConfigReplyParser.characters(
            "Terms of Use".toCharArray(), 0,
            "Terms of Use".length
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.TERMS_OF_USE,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.termsOfUse = "Terms of Use"
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml ProjectConfig has platform with name then expect matching ProjectConfig`() {
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfig.Fields.PLATFORMS,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, ProjectConfigReplyParser.PLATFORM_TAG,
            null, null
        )
        projectConfigReplyParser.startElement(
            null, PlatformInfo.Fields.NAME,
            null, null
        )
        projectConfigReplyParser.characters(
            "Platform Name".toCharArray(), 0,
            "Platform Name".length
        )
        projectConfigReplyParser.endElement(
            null, PlatformInfo.Fields.NAME,
            null
        )
        projectConfigReplyParser.startElement(
            null, USER_FRIENDLY_NAME,
            null, null
        )
        projectConfigReplyParser.characters(
            "Platform Friendly Name".toCharArray(), 0,
            "Platform Friendly Name".length
        )
        projectConfigReplyParser.endElement(
            null, USER_FRIENDLY_NAME,
            null
        )
        projectConfigReplyParser.startElement(
            null, PLAN_CLASS,
            null, null
        )
        projectConfigReplyParser.characters(
            "Plan Class".toCharArray(), 0,
            "Plan Class".length
        )
        projectConfigReplyParser.endElement(
            null, PLAN_CLASS,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PLATFORM_TAG,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfig.Fields.PLATFORMS,
            null
        )
        projectConfigReplyParser.endElement(
            null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
            null
        )
        expected.platforms.add(
            PlatformInfo(
                "Platform Name",
                "Platform Friendly Name",
                "Plan Class"
            )
        )
        Assert.assertEquals(expected, projectConfigReplyParser.projectConfig)
    }
}
