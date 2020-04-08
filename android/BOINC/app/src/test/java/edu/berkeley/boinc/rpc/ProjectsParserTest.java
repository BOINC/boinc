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

import java.util.Collections;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.doThrow;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class ProjectsParserTest {
    private static final String GUI_URL_NAME = "GUI URL Name";
    private static final String GUI_URL_DESCRIPTION = "GUI URL Description";
    private static final String GUI_URL_URL = "GUI URL URL";

    private static final String MASTER_URL = "Master URL";

    private ProjectsParser projectsParser;
    private Project expected;

    @Before
    public void setUp() {
        projectsParser = new ProjectsParser();
        expected = new Project();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectEmptyList() {
        mockStatic(Xml.class);

        assertTrue(ProjectsParser.parse(null).isEmpty());
    }

    @Test
    public void testParse_whenSAXExceptionIsThrown_thenExpectEmptyList() throws Exception {
        mockStatic(Xml.class);

        doThrow(new SAXException()).when(Xml.class, "parse", anyString(), any(ContentHandler.class));

        assertTrue(ProjectsParser.parse("").isEmpty());
    }

    @Test
    public void testParser_whenOnlyStartElementIsRun_thenExpectElementStarted() throws SAXException {
        projectsParser.startElement(null, "", null, null);

        assertTrue(projectsParser.mElementStarted);
    }

    @Test
    public void testParser_whenBothStartElementAndEndElementAreRun_thenExpectElementNotStarted()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        assertFalse(projectsParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectEmptyList() throws SAXException {
        projectsParser.startElement(null, "", null, null);

        assertTrue(projectsParser.getProjects().isEmpty());
    }

    @Test
    public void testParser_whenXmlProjectHasNoElements_thenExpectEmptyList()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        assertTrue(projectsParser.getProjects().isEmpty());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrl_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndGuiUrlWithName_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, RPCCommonTags.GUI_URL, null, null);
        projectsParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectsParser.characters(GUI_URL_NAME.toCharArray(), 0, GUI_URL_NAME.length());
        projectsParser.endElement(null, RPCCommonTags.NAME, null);
        projectsParser.endElement(null, RPCCommonTags.GUI_URL, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        final GuiUrl expectedGuiUrl = new GuiUrl();
        expected.setMasterURL(MASTER_URL);
        expectedGuiUrl.setName(GUI_URL_NAME);
        expected.getGuiURLs().add(expectedGuiUrl);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndGuiUrlWithDescription_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, RPCCommonTags.GUI_URL, null, null);
        projectsParser.startElement(null, RPCCommonTags.DESCRIPTION, null, null);
        projectsParser.characters(GUI_URL_DESCRIPTION.toCharArray(), 0, GUI_URL_DESCRIPTION.length());
        projectsParser.endElement(null, RPCCommonTags.DESCRIPTION, null);
        projectsParser.endElement(null, RPCCommonTags.GUI_URL, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        final GuiUrl expectedGuiUrl = new GuiUrl();
        expected.setMasterURL(MASTER_URL);
        expectedGuiUrl.setDescription(GUI_URL_DESCRIPTION);
        expected.getGuiURLs().add(expectedGuiUrl);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndGuiUrlWithUrl_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, RPCCommonTags.GUI_URL, null, null);
        projectsParser.startElement(null, RPCCommonTags.URL, null, null);
        projectsParser.characters(GUI_URL_URL.toCharArray(), 0, GUI_URL_URL.length());
        projectsParser.endElement(null, RPCCommonTags.URL, null);
        projectsParser.endElement(null, RPCCommonTags.GUI_URL, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        final GuiUrl expectedGuiUrl = new GuiUrl();
        expected.setMasterURL(MASTER_URL);
        expectedGuiUrl.setUrl(GUI_URL_URL);
        expected.getGuiURLs().add(expectedGuiUrl);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndGuiUrlWithAllAttributes_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, RPCCommonTags.GUI_URL, null, null);
        projectsParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectsParser.characters(GUI_URL_NAME.toCharArray(), 0, GUI_URL_NAME.length());
        projectsParser.endElement(null, RPCCommonTags.NAME, null);
        projectsParser.startElement(null, RPCCommonTags.DESCRIPTION, null, null);
        projectsParser.characters(GUI_URL_DESCRIPTION.toCharArray(), 0, GUI_URL_DESCRIPTION.length());
        projectsParser.endElement(null, RPCCommonTags.DESCRIPTION, null);
        projectsParser.startElement(null, RPCCommonTags.URL, null, null);
        projectsParser.characters(GUI_URL_URL.toCharArray(), 0, GUI_URL_URL.length());
        projectsParser.endElement(null, RPCCommonTags.URL, null);
        projectsParser.endElement(null, RPCCommonTags.GUI_URL, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        final GuiUrl expectedGuiUrl = new GuiUrl(GUI_URL_NAME, GUI_URL_DESCRIPTION, GUI_URL_URL);
        expected.setMasterURL(MASTER_URL);
        expected.getGuiURLs().add(expectedGuiUrl);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndProjectDir_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.PROJECT_DIR, null, null);
        projectsParser.characters("/path/to/project".toCharArray(), 0, "/path/to/project".length());
        projectsParser.endElement(null, Project.Fields.PROJECT_DIR, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setProjectDir("/path/to/project");

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndResourceShare_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.RESOURCE_SHARE, null, null);
        projectsParser.characters("1.5".toCharArray(), 0, 3);
        projectsParser.endElement(null, Project.Fields.RESOURCE_SHARE, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setResourceShare(1.5f);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndProjectName_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, RPCCommonTags.PROJECT_NAME, null, null);
        projectsParser.characters("Project Name".toCharArray(), 0, "Project Name".length());
        projectsParser.endElement(null, RPCCommonTags.PROJECT_NAME, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setProjectName("Project Name");

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndUserName_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.USER_NAME, null, null);
        projectsParser.characters("John".toCharArray(), 0, 4);
        projectsParser.endElement(null, Project.Fields.USER_NAME, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setUserName("John");

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndTeamName_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.TEAM_NAME, null, null);
        projectsParser.characters("Team Name".toCharArray(), 0, 9);
        projectsParser.endElement(null, Project.Fields.TEAM_NAME, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setTeamName("Team Name");

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndHostVenue_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.HOST_VENUE, null, null);
        projectsParser.characters("Host Venue".toCharArray(), 0, 10);
        projectsParser.endElement(null, Project.Fields.HOST_VENUE, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setHostVenue("Host Venue");

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndHostID_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.HOSTID, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.HOSTID, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setHostId(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndUserTotalCredit_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.USER_TOTAL_CREDIT, null, null);
        projectsParser.characters("100.5".toCharArray(), 0, 5);
        projectsParser.endElement(null, Project.Fields.USER_TOTAL_CREDIT, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setUserTotalCredit(100.5);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndUserExpAvgCredit_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.USER_EXPAVG_CREDIT, null, null);
        projectsParser.characters("50.5".toCharArray(), 0, 4);
        projectsParser.endElement(null, Project.Fields.USER_EXPAVG_CREDIT, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setUserExpAvgCredit(50.5);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndHostTotalCredit_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.HOST_TOTAL_CREDIT, null, null);
        projectsParser.characters("200.5".toCharArray(), 0, 5);
        projectsParser.endElement(null, Project.Fields.HOST_TOTAL_CREDIT, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setHostTotalCredit(200.5);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndHostExpAvgCredit_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.HOST_EXPAVG_CREDIT, null, null);
        projectsParser.characters("100.5".toCharArray(), 0, 5);
        projectsParser.endElement(null, Project.Fields.HOST_EXPAVG_CREDIT, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setHostExpAvgCredit(100.5);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoOfRpcFailures_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.NRPC_FAILURES, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.NRPC_FAILURES, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setNoOfRPCFailures(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndMasterFetchFailures_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.MASTER_FETCH_FAILURES, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.MASTER_FETCH_FAILURES, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setMasterFetchFailures(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndMinRpcTime_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.MIN_RPC_TIME, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.MIN_RPC_TIME, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setMinRPCTime(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndDownloadBackoff_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.DOWNLOAD_BACKOFF, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.DOWNLOAD_BACKOFF, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setDownloadBackoff(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndUploadBackoff_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.UPLOAD_BACKOFF, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.UPLOAD_BACKOFF, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setUploadBackoff(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndShortTermDebt_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, ProjectsParser.SHORT_TERM_DEBT_TAG, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, ProjectsParser.SHORT_TERM_DEBT_TAG, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setCpuShortTermDebt(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndLongTermDebt_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, ProjectsParser.LONG_TERM_DEBT_TAG, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, ProjectsParser.LONG_TERM_DEBT_TAG, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setCpuLongTermDebt(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndCpuBackoffTime_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.CPU_BACKOFF_TIME, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.CPU_BACKOFF_TIME, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setCpuBackoffTime(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndCpuBackoffInterval_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.CPU_BACKOFF_INTERVAL, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.CPU_BACKOFF_INTERVAL, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setCpuBackoffInterval(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndCudaDebt_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.CUDA_DEBT, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.CUDA_DEBT, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setCudaDebt(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndCudaShortTermDebt_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.CUDA_SHORT_TERM_DEBT, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.CUDA_SHORT_TERM_DEBT, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setCudaShortTermDebt(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndCudaBackoffTime_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.CUDA_BACKOFF_TIME, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.CUDA_BACKOFF_TIME, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setCudaBackoffTime(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndCudaBackoffInterval_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.CUDA_BACKOFF_INTERVAL, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.CUDA_BACKOFF_INTERVAL, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setCudaBackoffInterval(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndAtiDebt_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.ATI_DEBT, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ATI_DEBT, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setAtiDebt(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndAtiShortTermDebt_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.ATI_SHORT_TERM_DEBT, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ATI_SHORT_TERM_DEBT, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setAtiShortTermDebt(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndAtiBackoffTime_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.ATI_BACKOFF_TIME, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ATI_BACKOFF_TIME, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setAtiBackoffTime(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndAtiBackoffInterval_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.ATI_BACKOFF_INTERVAL, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ATI_BACKOFF_INTERVAL, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setAtiBackoffInterval(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndDurationCorrectionFactor_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.DURATION_CORRECTION_FACTOR, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.DURATION_CORRECTION_FACTOR, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setDurationCorrectionFactor(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndMasterUrlFetchPendingIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.MASTER_URL_FETCH_PENDING, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.MASTER_URL_FETCH_PENDING, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setMasterURLFetchPending(false);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndMasterUrlFetchPendingIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.MASTER_URL_FETCH_PENDING, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.MASTER_URL_FETCH_PENDING, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setMasterURLFetchPending(true);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndSchedRpcPending_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.SCHED_RPC_PENDING, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.SCHED_RPC_PENDING, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setScheduledRPCPending(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNonCpuIntensiveIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setNonCPUIntensive(false);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNonCpuIntensiveIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setNonCPUIntensive(true);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndSuspendedViaGuiIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.SUSPENDED_VIA_GUI, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.SUSPENDED_VIA_GUI, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setSuspendedViaGUI(false);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndSuspendedViaGuiIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.SUSPENDED_VIA_GUI, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.SUSPENDED_VIA_GUI, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setSuspendedViaGUI(true);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndDontRequestMoreWorkIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.DONT_REQUEST_MORE_WORK, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.DONT_REQUEST_MORE_WORK, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setDoNotRequestMoreWork(false);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndDontRequestMoreWorkIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.DONT_REQUEST_MORE_WORK, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.DONT_REQUEST_MORE_WORK, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setDoNotRequestMoreWork(true);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndSchedulerRpcInProgressIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.SCHEDULER_RPC_IN_PROGRESS, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.SCHEDULER_RPC_IN_PROGRESS, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setSchedulerRPCInProgress(false);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndSchedulerRpcInProgressIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.SCHEDULER_RPC_IN_PROGRESS, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.SCHEDULER_RPC_IN_PROGRESS, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setSchedulerRPCInProgress(true);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndAttachedViaAcctMgrIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.ATTACHED_VIA_ACCT_MGR, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ATTACHED_VIA_ACCT_MGR, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setAttachedViaAcctMgr(false);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndAttachedViaAcctMgrIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.ATTACHED_VIA_ACCT_MGR, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ATTACHED_VIA_ACCT_MGR, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setAttachedViaAcctMgr(true);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndDetachWhenDoneIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.DETACH_WHEN_DONE, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.DETACH_WHEN_DONE, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setDetachWhenDone(false);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndDetachWhenDoneIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.DETACH_WHEN_DONE, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.DETACH_WHEN_DONE, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setDetachWhenDone(true);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndEndedIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.ENDED, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ENDED, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setEnded(false);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndEndedIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.ENDED, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ENDED, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setEnded(true);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndTrickleUpPendingIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.TRICKLE_UP_PENDING, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.TRICKLE_UP_PENDING, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setTrickleUpPending(false);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndTrickleUpPendingIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.TRICKLE_UP_PENDING, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.TRICKLE_UP_PENDING, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setTrickleUpPending(true);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndProjectFilesDownloadedTime_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.PROJECT_FILES_DOWNLOADED_TIME, null, null);
        projectsParser.characters("10".toCharArray(), 0, 2);
        projectsParser.endElement(null, Project.Fields.PROJECT_FILES_DOWNLOADED_TIME, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setProjectFilesDownloadedTime(10);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndLastRpcTime_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.LAST_RPC_TIME, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.LAST_RPC_TIME, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setLastRPCTime(1);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoCpuPrefIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.NO_CPU_PREF, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.NO_CPU_PREF, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setNoCPUPref(false);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoCpuPrefIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.NO_CPU_PREF, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.NO_CPU_PREF, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setNoCPUPref(true);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoCudaPrefIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.NO_CUDA_PREF, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.NO_CUDA_PREF, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setNoCUDAPref(false);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoCudaPrefIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.NO_CUDA_PREF, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.NO_CUDA_PREF, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setNoCUDAPref(true);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoAtiPrefIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.NO_ATI_PREF, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.NO_ATI_PREF, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setNoATIPref(false);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoAtiPrefIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectsParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        projectsParser.startElement(null, Project.Fields.NO_ATI_PREF, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.NO_ATI_PREF, null);
        projectsParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setMasterURL(MASTER_URL);
        expected.setNoATIPref(true);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }
}
