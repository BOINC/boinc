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

import java.util.Collections;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
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

        assertEquals(Collections.emptyList(), ProjectsParser.parse(null));
    }

    @Test
    public void testParser_whenOnlyStartElementIsRun_thenExpectElementStarted() throws SAXException {
        projectsParser.startElement(null, "", null, null);

        assertTrue(projectsParser.mElementStarted);
    }

    @Test
    public void testParser_whenBothStartElementAndEndElementAreRun_thenExpectElementNotStarted()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

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
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        assertTrue(projectsParser.getProjects().isEmpty());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrl_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndGuiUrlWithName_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, ProjectsParser.GUI_URL_TAG, null, null);
        projectsParser.startElement(null, GuiUrl.Fields.NAME, null, null);
        projectsParser.characters(GUI_URL_NAME.toCharArray(), 0, GUI_URL_NAME.length());
        projectsParser.endElement(null, GuiUrl.Fields.NAME, null);
        projectsParser.endElement(null, ProjectsParser.GUI_URL_TAG, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        final GuiUrl expectedGuiUrl = new GuiUrl();
        expected.master_url = MASTER_URL;
        expectedGuiUrl.setName(GUI_URL_NAME);
        expected.gui_urls.add(expectedGuiUrl);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndGuiUrlWithDescription_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, ProjectsParser.GUI_URL_TAG, null, null);
        projectsParser.startElement(null, GuiUrl.Fields.DESCRIPTION, null, null);
        projectsParser.characters(GUI_URL_DESCRIPTION.toCharArray(), 0, GUI_URL_DESCRIPTION.length());
        projectsParser.endElement(null, GuiUrl.Fields.DESCRIPTION, null);
        projectsParser.endElement(null, ProjectsParser.GUI_URL_TAG, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        final GuiUrl expectedGuiUrl = new GuiUrl();
        expected.master_url = MASTER_URL;
        expectedGuiUrl.setDescription(GUI_URL_DESCRIPTION);
        expected.gui_urls.add(expectedGuiUrl);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndGuiUrlWithUrl_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, ProjectsParser.GUI_URL_TAG, null, null);
        projectsParser.startElement(null, GuiUrl.Fields.URL, null, null);
        projectsParser.characters(GUI_URL_URL.toCharArray(), 0, GUI_URL_URL.length());
        projectsParser.endElement(null, GuiUrl.Fields.URL, null);
        projectsParser.endElement(null, ProjectsParser.GUI_URL_TAG, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        final GuiUrl expectedGuiUrl = new GuiUrl();
        expected.master_url = MASTER_URL;
        expectedGuiUrl.setUrl(GUI_URL_URL);
        expected.gui_urls.add(expectedGuiUrl);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndGuiUrlWithAllAttributes_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, ProjectsParser.GUI_URL_TAG, null, null);
        projectsParser.startElement(null, GuiUrl.Fields.NAME, null, null);
        projectsParser.characters(GUI_URL_NAME.toCharArray(), 0, GUI_URL_NAME.length());
        projectsParser.endElement(null, GuiUrl.Fields.NAME, null);
        projectsParser.startElement(null, GuiUrl.Fields.DESCRIPTION, null, null);
        projectsParser.characters(GUI_URL_DESCRIPTION.toCharArray(), 0, GUI_URL_DESCRIPTION.length());
        projectsParser.endElement(null, GuiUrl.Fields.DESCRIPTION, null);
        projectsParser.startElement(null, GuiUrl.Fields.URL, null, null);
        projectsParser.characters(GUI_URL_URL.toCharArray(), 0, GUI_URL_URL.length());
        projectsParser.endElement(null, GuiUrl.Fields.URL, null);
        projectsParser.endElement(null, ProjectsParser.GUI_URL_TAG, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        final GuiUrl expectedGuiUrl = new GuiUrl(GUI_URL_NAME, GUI_URL_DESCRIPTION, GUI_URL_URL);
        expected.master_url = MASTER_URL;
        expected.gui_urls.add(expectedGuiUrl);

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndProjectDir_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.project_dir, null, null);
        projectsParser.characters("/path/to/project".toCharArray(), 0, "/path/to/project".length());
        projectsParser.endElement(null, Project.Fields.project_dir, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.project_dir = "/path/to/project";

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndResourceShare_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.resource_share, null, null);
        projectsParser.characters("1.5".toCharArray(), 0, 3);
        projectsParser.endElement(null, Project.Fields.resource_share, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.resource_share = 1.5f;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndProjectName_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.project_name, null, null);
        projectsParser.characters("Project Name".toCharArray(), 0, "Project Name".length());
        projectsParser.endElement(null, Project.Fields.project_name, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.project_name = "Project Name";

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndUserName_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.user_name, null, null);
        projectsParser.characters("John".toCharArray(), 0, 4);
        projectsParser.endElement(null, Project.Fields.user_name, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.user_name = "John";

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndTeamName_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.team_name, null, null);
        projectsParser.characters("Team Name".toCharArray(), 0, 9);
        projectsParser.endElement(null, Project.Fields.team_name, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.team_name = "Team Name";

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndHostVenue_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.host_venue, null, null);
        projectsParser.characters("Host Venue".toCharArray(), 0, 10);
        projectsParser.endElement(null, Project.Fields.host_venue, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.host_venue = "Host Venue";

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndHostID_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.hostid, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.hostid, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.hostid = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndUserTotalCredit_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.user_total_credit, null, null);
        projectsParser.characters("100.5".toCharArray(), 0, 5);
        projectsParser.endElement(null, Project.Fields.user_total_credit, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.user_total_credit = 100.5;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndUserExpAvgCredit_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.user_expavg_credit, null, null);
        projectsParser.characters("50.5".toCharArray(), 0, 4);
        projectsParser.endElement(null, Project.Fields.user_expavg_credit, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.user_expavg_credit = 50.5;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndHostTotalCredit_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.host_total_credit, null, null);
        projectsParser.characters("200.5".toCharArray(), 0, 5);
        projectsParser.endElement(null, Project.Fields.host_total_credit, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.host_total_credit = 200.5;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndHostExpAvgCredit_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.host_expavg_credit, null, null);
        projectsParser.characters("100.5".toCharArray(), 0, 5);
        projectsParser.endElement(null, Project.Fields.host_expavg_credit, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.host_expavg_credit = 100.5;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoOfRpcFailures_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.nrpc_failures, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.nrpc_failures, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.nrpc_failures = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndMasterFetchFailures_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.master_fetch_failures, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.master_fetch_failures, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.master_fetch_failures = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndMinRpcTime_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.min_rpc_time, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.min_rpc_time, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.min_rpc_time = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndDownloadBackoff_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.download_backoff, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.download_backoff, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.download_backoff = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndUploadBackoff_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.upload_backoff, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.upload_backoff, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.upload_backoff = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndShortTermDebt_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, ProjectsParser.SHORT_TERM_DEBT_TAG, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, ProjectsParser.SHORT_TERM_DEBT_TAG, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.cpu_short_term_debt = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndLongTermDebt_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, ProjectsParser.LONG_TERM_DEBT_TAG, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, ProjectsParser.LONG_TERM_DEBT_TAG, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.cpu_long_term_debt = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndCpuBackoffTime_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.cpu_backoff_time, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.cpu_backoff_time, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.cpu_backoff_time = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndCpuBackoffInterval_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.cpu_backoff_interval, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.cpu_backoff_interval, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.cpu_backoff_interval = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndCudaDebt_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.cuda_debt, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.cuda_debt, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.cuda_debt = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndCudaShortTermDebt_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.cuda_short_term_debt, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.cuda_short_term_debt, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.cuda_short_term_debt = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndCudaBackoffTime_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.cuda_backoff_time, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.cuda_backoff_time, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.cuda_backoff_time = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndCudaBackoffInterval_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.cuda_backoff_interval, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.cuda_backoff_interval, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.cuda_backoff_interval = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndAtiDebt_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.ati_debt, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ati_debt, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.ati_debt = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndAtiShortTermDebt_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.ati_short_term_debt, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ati_short_term_debt, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.ati_short_term_debt = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndAtiBackoffTime_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.ati_backoff_time, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ati_backoff_time, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.ati_backoff_time = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndAtiBackoffInterval_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.ati_backoff_interval, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ati_backoff_interval, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.ati_backoff_interval = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndDurationCorrectionFactor_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.duration_correction_factor, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.duration_correction_factor, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.duration_correction_factor = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndMasterUrlFetchPendingIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.master_url_fetch_pending, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.master_url_fetch_pending, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.master_url_fetch_pending = false;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndMasterUrlFetchPendingIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.master_url_fetch_pending, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.master_url_fetch_pending, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.master_url_fetch_pending = true;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndSchedRpcPending_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.sched_rpc_pending, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.sched_rpc_pending, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.sched_rpc_pending = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNonCpuIntensiveIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.non_cpu_intensive, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.non_cpu_intensive, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.non_cpu_intensive = false;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNonCpuIntensiveIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.non_cpu_intensive, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.non_cpu_intensive, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.non_cpu_intensive = true;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndSuspendedViaGuiIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.suspended_via_gui, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.suspended_via_gui, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.suspended_via_gui = false;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndSuspendedViaGuiIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.suspended_via_gui, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.suspended_via_gui, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.suspended_via_gui = true;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndDontRequestMoreWorkIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.dont_request_more_work, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.dont_request_more_work, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.dont_request_more_work = false;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndDontRequestMoreWorkIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.dont_request_more_work, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.dont_request_more_work, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.dont_request_more_work = true;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndSchedulerRpcInProgressIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.scheduler_rpc_in_progress, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.scheduler_rpc_in_progress, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.scheduler_rpc_in_progress = false;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndSchedulerRpcInProgressIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.scheduler_rpc_in_progress, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.scheduler_rpc_in_progress, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.scheduler_rpc_in_progress = true;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndAttachedViaAcctMgrIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.attached_via_acct_mgr, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.attached_via_acct_mgr, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.attached_via_acct_mgr = false;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndAttachedViaAcctMgrIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.attached_via_acct_mgr, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.attached_via_acct_mgr, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.attached_via_acct_mgr = true;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndDetachWhenDoneIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.detach_when_done, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.detach_when_done, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.detach_when_done = false;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndDetachWhenDoneIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.detach_when_done, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.detach_when_done, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.detach_when_done = true;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndEndedIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.ended, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ended, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.ended = false;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndEndedIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.ended, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.ended, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.ended = true;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndTrickleUpPendingIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.trickle_up_pending, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.trickle_up_pending, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.trickle_up_pending = false;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndTrickleUpPendingIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.trickle_up_pending, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.trickle_up_pending, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.trickle_up_pending = true;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndProjectFilesDownloadedTime_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.project_files_downloaded_time, null, null);
        projectsParser.characters("10".toCharArray(), 0, 2);
        projectsParser.endElement(null, Project.Fields.project_files_downloaded_time, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.project_files_downloaded_time = 10;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndLastRpcTime_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.last_rpc_time, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.last_rpc_time, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.last_rpc_time = 1;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoCpuPrefIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.no_cpu_pref, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.no_cpu_pref, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.no_cpu_pref = false;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoCpuPrefIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.no_cpu_pref, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.no_cpu_pref, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.no_cpu_pref = true;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoCudaPrefIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.no_cuda_pref, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.no_cuda_pref, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.no_cuda_pref = false;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoCudaPrefIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.no_cuda_pref, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.no_cuda_pref, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.no_cuda_pref = true;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoAtiPrefIs0_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.no_ati_pref, null, null);
        projectsParser.characters("0".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.no_ati_pref, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.no_ati_pref = false;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }

    @Test
    public void testParser_whenXmlProjectHasMasterUrlAndNoAtiPrefIs1_thenExpectListWithMatchingProject()
            throws SAXException {
        projectsParser.startElement(null, ProjectsParser.PROJECT_TAG, null, null);
        projectsParser.startElement(null, Project.Fields.master_url, null, null);
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length());
        projectsParser.endElement(null, Project.Fields.master_url, null);
        projectsParser.startElement(null, Project.Fields.no_ati_pref, null, null);
        projectsParser.characters("1".toCharArray(), 0, 1);
        projectsParser.endElement(null, Project.Fields.no_ati_pref, null);
        projectsParser.endElement(null, ProjectsParser.PROJECT_TAG, null);

        expected.master_url = MASTER_URL;
        expected.no_ati_pref = true;

        assertEquals(Collections.singletonList(expected), projectsParser.getProjects());
    }
}