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

import kotlin.UninitializedPropertyAccessException;

import static org.junit.Assert.*;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class ProjectConfigReplyParserTest {
    private ProjectConfigReplyParser projectConfigReplyParser;
    private ProjectConfig expected;

    @Before
    public void setUp() {
        projectConfigReplyParser = new ProjectConfigReplyParser();
        expected = new ProjectConfig();
    }

    @Test(expected = IllegalArgumentException.class)
    public void testParse_whenRpcStringIsNull_thenExpectIllegalArgumentException() {
        mockStatic(Xml.class);

        ProjectConfigReplyParser.parse(null);
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParse_whenRpcStringIsEmpty_thenExpectUninitializedPropertyAccessException() {
        mockStatic(Xml.class);

        ProjectConfigReplyParser.parse("");
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectElementStarted() throws SAXException {
        projectConfigReplyParser.startElement(null, "", null, null);

        assertTrue(projectConfigReplyParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsProjectConfigTag_thenExpectElementNotStarted()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        assertFalse(projectConfigReplyParser.mElementStarted);
    }

    @Test
    public void testParser_whenXmlProjectConfigHasName_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectConfigReplyParser.characters("Project Config".toCharArray(), 0,
                                            "Project Config".length());
        projectConfigReplyParser.endElement(null, RPCCommonTags.NAME, null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setName("Project Config");

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasMasterUrl_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, RPCCommonTags.MASTER_URL,
                                              null, null);
        projectConfigReplyParser.characters("Master URL".toCharArray(), 0, "Master URL".length());
        projectConfigReplyParser.endElement(null, RPCCommonTags.MASTER_URL,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setMasterUrl("Master URL");

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasWebRpcUrlBase_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.WEB_RPC_URL_BASE,
                                              null, null);
        projectConfigReplyParser.characters("Web RPC URL Base".toCharArray(), 0,
                                            "Web RPC URL Base".length());
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.WEB_RPC_URL_BASE,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setWebRpcUrlBase("Web RPC URL Base");

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasLocalRevision_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.LOCAL_REVISION,
                                              null, null);
        projectConfigReplyParser.characters("Local Revision".toCharArray(), 0,
                                            "Local Revision".length());
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.LOCAL_REVISION,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setLocalRevision("Local Revision");

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasInvalidMinPwdLength_thenExpectDefaultProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.MIN_PWD_LENGTH,
                                              null, null);
        projectConfigReplyParser.characters("Eight".toCharArray(), 0, 5);
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.MIN_PWD_LENGTH,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasMinPwdLength_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.MIN_PWD_LENGTH,
                                              null, null);
        projectConfigReplyParser.characters("8".toCharArray(), 0, 1);
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.MIN_PWD_LENGTH,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setMinPwdLength(8);

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasUserNameTag_thenExpectProjectConfigWithUsesNameTrue()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.USER_NAME_TAG,
                                              null, null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.USER_NAME_TAG,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setUsesName(true);

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasUsesUserNameTag_thenExpectProjectConfigWithUsesNameTrue()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.USES_USER_NAME_TAG,
                                              null, null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.USES_USER_NAME_TAG,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setUsesName(true);

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasWebStopped0_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.WEB_STOPPED,
                                              null, null);
        projectConfigReplyParser.characters("0".toCharArray(), 0, 1);
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.WEB_STOPPED,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setWebStopped(false);

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasWebStopped1_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.WEB_STOPPED,
                                              null, null);
        projectConfigReplyParser.characters("1".toCharArray(), 0, 1);
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.WEB_STOPPED,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setWebStopped(true);

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasSchedulerStopped0_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.SCHEDULER_STOPPED,
                                              null, null);
        projectConfigReplyParser.characters("0".toCharArray(), 0, 1);
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.SCHEDULER_STOPPED,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setSchedulerStopped(false);

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasSchedulerStopped1_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.SCHEDULER_STOPPED,
                                              null, null);
        projectConfigReplyParser.characters("1".toCharArray(), 0, 1);
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.SCHEDULER_STOPPED,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setSchedulerStopped(true);

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasClientAccountCreationDisabledTag_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.CLIENT_ACCOUNT_CREATION_DISABLED,
                                              null, null);
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.CLIENT_ACCOUNT_CREATION_DISABLED,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setClientAccountCreationDisabled(true);

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasMinClientVersion_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.MIN_CLIENT_VERSION,
                                              null, null);
        projectConfigReplyParser.characters("1".toCharArray(), 0, 1);
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.MIN_CLIENT_VERSION,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setMinClientVersion(1);

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasRpcPrefix_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.RPC_PREFIX,
                                              null, null);
        projectConfigReplyParser.characters("RPC Prefix".toCharArray(), 0, "RPC Prefix".length());
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.RPC_PREFIX,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setRpcPrefix("RPC Prefix");

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasErrorNum_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, RPCCommonTags.ERROR_NUM, null, null);
        projectConfigReplyParser.characters("1".toCharArray(), 0, 1);
        projectConfigReplyParser.endElement(null, RPCCommonTags.ERROR_NUM, null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setErrorNum(1);

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasAccountManagerTag_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.ACCOUNT_MANAGER,
                                              null, null);
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.ACCOUNT_MANAGER,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setAccountManager(true);

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasTermsOfUse_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.TERMS_OF_USE,
                                              null, null);
        projectConfigReplyParser.characters("Terms of Use".toCharArray(), 0,
                                            "Terms of Use".length());
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.TERMS_OF_USE,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.setTermsOfUse("Terms of Use");

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }

    @Test
    public void testParser_whenXmlProjectConfigHasPlatformWithName_thenExpectMatchingProjectConfig()
            throws SAXException {
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfig.Fields.PLATFORMS,
                                              null, null);
        projectConfigReplyParser.startElement(null, ProjectConfigReplyParser.PLATFORM_TAG,
                                              null, null);
        projectConfigReplyParser.startElement(null, PlatformInfo.Fields.NAME,
                                              null, null);
        projectConfigReplyParser.characters("Platform Name".toCharArray(), 0,
                                            "Platform Name".length());
        projectConfigReplyParser.endElement(null, PlatformInfo.Fields.NAME,
                                            null);
        projectConfigReplyParser.startElement(null, RPCCommonTags.USER_FRIENDLY_NAME,
                                              null, null);
        projectConfigReplyParser.characters("Platform Friendly Name".toCharArray(), 0,
                                            "Platform Friendly Name".length());
        projectConfigReplyParser.endElement(null, RPCCommonTags.USER_FRIENDLY_NAME,
                                            null);
        projectConfigReplyParser.startElement(null, RPCCommonTags.PLAN_CLASS,
                                              null, null);
        projectConfigReplyParser.characters("Plan Class".toCharArray(), 0,
                                            "Plan Class".length());
        projectConfigReplyParser.endElement(null, RPCCommonTags.PLAN_CLASS,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PLATFORM_TAG,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfig.Fields.PLATFORMS,
                                            null);
        projectConfigReplyParser.endElement(null, ProjectConfigReplyParser.PROJECT_CONFIG_TAG,
                                            null);

        expected.getPlatforms().add(new PlatformInfo("Platform Name",
                                                "Platform Friendly Name",
                                                "Plan Class"));

        assertEquals(expected, projectConfigReplyParser.getProjectConfig());
    }
}
