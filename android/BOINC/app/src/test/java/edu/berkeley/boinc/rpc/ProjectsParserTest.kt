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
import edu.berkeley.boinc.rpc.ProjectsParser.Companion.parse
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
class ProjectsParserTest {
    private lateinit var projectsParser: ProjectsParser
    private lateinit var expected: Project
    @Before
    fun setUp() {
        projectsParser = ProjectsParser()
        expected = Project()
    }

    @Test
    fun `When Rpc string is null then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        Assert.assertTrue(parse(null).isEmpty())
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertTrue(parse("").isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When only start element is run then expect element started`() {
        projectsParser.startElement(null, "", null, null)
        Assert.assertTrue(projectsParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When both start element and end element are run then expect element not started`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.endElement(null, PROJECT, null)
        Assert.assertFalse(projectsParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect empty list`() {
        projectsParser.startElement(null, "", null, null)
        Assert.assertTrue(projectsParser.projects.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has no elements then expect empty list`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.endElement(null, PROJECT, null)
        Assert.assertTrue(projectsParser.projects.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and gui_url_with_name then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, GUI_URL, null, null)
        projectsParser.startElement(null, NAME, null, null)
        projectsParser.characters(GUI_URL_NAME.toCharArray(), 0, GUI_URL_NAME.length)
        projectsParser.endElement(null, NAME, null)
        projectsParser.endElement(null, GUI_URL, null)
        projectsParser.endElement(null, PROJECT, null)
        val expectedGuiUrl = GuiUrl()
        expected.masterURL = MASTER_URL
        expectedGuiUrl.name = GUI_URL_NAME
        expected.guiURLs.add(expectedGuiUrl)
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and gui_url with description then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, GUI_URL, null, null)
        projectsParser.startElement(null, DESCRIPTION, null, null)
        projectsParser.characters(
            GUI_URL_DESCRIPTION.toCharArray(),
            0,
            GUI_URL_DESCRIPTION.length
        )
        projectsParser.endElement(null, DESCRIPTION, null)
        projectsParser.endElement(null, GUI_URL, null)
        projectsParser.endElement(null, PROJECT, null)
        val expectedGuiUrl = GuiUrl()
        expected.masterURL = MASTER_URL
        expectedGuiUrl.description = GUI_URL_DESCRIPTION
        expected.guiURLs.add(expectedGuiUrl)
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and gui_url with url then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, GUI_URL, null, null)
        projectsParser.startElement(null, URL, null, null)
        projectsParser.characters(GUI_URL_URL.toCharArray(), 0, GUI_URL_URL.length)
        projectsParser.endElement(null, URL, null)
        projectsParser.endElement(null, GUI_URL, null)
        projectsParser.endElement(null, PROJECT, null)
        val expectedGuiUrl = GuiUrl()
        expected.masterURL = MASTER_URL
        expectedGuiUrl.url = GUI_URL_URL
        expected.guiURLs.add(expectedGuiUrl)
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and gui_url with all attributes then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, GUI_URL, null, null)
        projectsParser.startElement(null, NAME, null, null)
        projectsParser.characters(GUI_URL_NAME.toCharArray(), 0, GUI_URL_NAME.length)
        projectsParser.endElement(null, NAME, null)
        projectsParser.startElement(null, DESCRIPTION, null, null)
        projectsParser.characters(
            GUI_URL_DESCRIPTION.toCharArray(),
            0,
            GUI_URL_DESCRIPTION.length
        )
        projectsParser.endElement(null, DESCRIPTION, null)
        projectsParser.startElement(null, URL, null, null)
        projectsParser.characters(GUI_URL_URL.toCharArray(), 0, GUI_URL_URL.length)
        projectsParser.endElement(null, URL, null)
        projectsParser.endElement(null, GUI_URL, null)
        projectsParser.endElement(null, PROJECT, null)
        val expectedGuiUrl = GuiUrl(GUI_URL_NAME, GUI_URL_DESCRIPTION, GUI_URL_URL)
        expected.masterURL = MASTER_URL
        expected.guiURLs.add(expectedGuiUrl)
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and project_dir then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.PROJECT_DIR, null, null)
        projectsParser.characters("/path/to/project".toCharArray(), 0, "/path/to/project".length)
        projectsParser.endElement(null, Project.Fields.PROJECT_DIR, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.projectDir = "/path/to/project"
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and resource_share then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.RESOURCE_SHARE, null, null)
        projectsParser.characters("1.5".toCharArray(), 0, 3)
        projectsParser.endElement(null, Project.Fields.RESOURCE_SHARE, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.resourceShare = 1.5f
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and disk_usage then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.DISK_USAGE, null, null)
        projectsParser.characters("1024.0".toCharArray(), 0, 6)
        projectsParser.endElement(null, Project.Fields.DISK_USAGE, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.diskUsage = 1024.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and project_name then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, PROJECT_NAME, null, null)
        projectsParser.characters("Project Name".toCharArray(), 0, "Project Name".length)
        projectsParser.endElement(null, PROJECT_NAME, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.projectName = "Project Name"
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and user_name then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.USER_NAME, null, null)
        projectsParser.characters("John".toCharArray(), 0, 4)
        projectsParser.endElement(null, Project.Fields.USER_NAME, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.userName = "John"
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and team_name then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.TEAM_NAME, null, null)
        projectsParser.characters("Team Name".toCharArray(), 0, 9)
        projectsParser.endElement(null, Project.Fields.TEAM_NAME, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.teamName = "Team Name"
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and host_venue then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.HOST_VENUE, null, null)
        projectsParser.characters("Host Venue".toCharArray(), 0, 10)
        projectsParser.endElement(null, Project.Fields.HOST_VENUE, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.hostVenue = "Host Venue"
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and host_id then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.HOSTID, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.HOSTID, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.hostId = 1
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and user_total_credit then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.USER_TOTAL_CREDIT, null, null)
        projectsParser.characters("100.5".toCharArray(), 0, 5)
        projectsParser.endElement(null, Project.Fields.USER_TOTAL_CREDIT, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.userTotalCredit = 100.5
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and user_exp_avg_credit then expect list with matching project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.USER_EXPAVG_CREDIT, null, null)
        projectsParser.characters("50.5".toCharArray(), 0, 4)
        projectsParser.endElement(null, Project.Fields.USER_EXPAVG_CREDIT, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.userExpAvgCredit = 50.5
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and host_total_credit then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.HOST_TOTAL_CREDIT, null, null)
        projectsParser.characters("200.5".toCharArray(), 0, 5)
        projectsParser.endElement(null, Project.Fields.HOST_TOTAL_CREDIT, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.hostTotalCredit = 200.5
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and host_exp_avg_credit then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.HOST_EXPAVG_CREDIT, null, null)
        projectsParser.characters("100.5".toCharArray(), 0, 5)
        projectsParser.endElement(null, Project.Fields.HOST_EXPAVG_CREDIT, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.hostExpAvgCredit = 100.5
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and no_of_rpc_failures then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.NRPC_FAILURES, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.NRPC_FAILURES, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.noOfRPCFailures = 1
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and master_fetch_failures then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.MASTER_FETCH_FAILURES, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.MASTER_FETCH_FAILURES, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.masterFetchFailures = 1
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and min_rpc_time then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.MIN_RPC_TIME, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.MIN_RPC_TIME, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.minRPCTime = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and download_backoff then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.DOWNLOAD_BACKOFF, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.DOWNLOAD_BACKOFF, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.downloadBackoff = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and upload_backoff then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.UPLOAD_BACKOFF, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.UPLOAD_BACKOFF, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.uploadBackoff = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and short_term_debt then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, ProjectsParser.SHORT_TERM_DEBT_TAG, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, ProjectsParser.SHORT_TERM_DEBT_TAG, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.cpuShortTermDebt = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and long_term_debt then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, ProjectsParser.LONG_TERM_DEBT_TAG, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, ProjectsParser.LONG_TERM_DEBT_TAG, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.cpuLongTermDebt = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and cpu_backoff_time then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.CPU_BACKOFF_TIME, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.CPU_BACKOFF_TIME, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.cpuBackoffTime = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and cpu_backoff_interval then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.CPU_BACKOFF_INTERVAL, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.CPU_BACKOFF_INTERVAL, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.cpuBackoffInterval = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and cuda_debt then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.CUDA_DEBT, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.CUDA_DEBT, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.cudaDebt = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and cuda_short_term_debt then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.CUDA_SHORT_TERM_DEBT, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.CUDA_SHORT_TERM_DEBT, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.cudaShortTermDebt = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and cuda_backoff_time then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.CUDA_BACKOFF_TIME, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.CUDA_BACKOFF_TIME, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.cudaBackoffTime = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and cuda_backoff_interval then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.CUDA_BACKOFF_INTERVAL, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.CUDA_BACKOFF_INTERVAL, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.cudaBackoffInterval = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and ati_debt then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.ATI_DEBT, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.ATI_DEBT, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.atiDebt = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and ati_short_term_debt then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.ATI_SHORT_TERM_DEBT, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.ATI_SHORT_TERM_DEBT, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.atiShortTermDebt = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and ati_backoff_time then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.ATI_BACKOFF_TIME, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.ATI_BACKOFF_TIME, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.atiBackoffTime = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and ati_backoff_interval then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.ATI_BACKOFF_INTERVAL, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.ATI_BACKOFF_INTERVAL, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.atiBackoffInterval = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and duration_correction_factor then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.DURATION_CORRECTION_FACTOR, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.DURATION_CORRECTION_FACTOR, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.durationCorrectionFactor = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and master_url_fetch_pending is 0 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.MASTER_URL_FETCH_PENDING, null, null)
        projectsParser.characters("0".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.MASTER_URL_FETCH_PENDING, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.masterURLFetchPending = false
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and master_url_fetch_pending is 1 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.MASTER_URL_FETCH_PENDING, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.MASTER_URL_FETCH_PENDING, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.masterURLFetchPending = true
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and sched_rpc_pending then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.SCHED_RPC_PENDING, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.SCHED_RPC_PENDING, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.scheduledRPCPending = 1
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and non_cpu_intensive is 0 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, NON_CPU_INTENSIVE, null, null)
        projectsParser.characters("0".toCharArray(), 0, 1)
        projectsParser.endElement(null, NON_CPU_INTENSIVE, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.nonCPUIntensive = false
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and non_cpu_intensive is 1 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, NON_CPU_INTENSIVE, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, NON_CPU_INTENSIVE, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.nonCPUIntensive = true
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and suspended_via_gui is 0 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.SUSPENDED_VIA_GUI, null, null)
        projectsParser.characters("0".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.SUSPENDED_VIA_GUI, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.suspendedViaGUI = false
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and suspended_via_gui is 1 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.SUSPENDED_VIA_GUI, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.SUSPENDED_VIA_GUI, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.suspendedViaGUI = true
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and dont_request_more_work is 0 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.DONT_REQUEST_MORE_WORK, null, null)
        projectsParser.characters("0".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.DONT_REQUEST_MORE_WORK, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.doNotRequestMoreWork = false
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and dont_request_more_work is 1 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.DONT_REQUEST_MORE_WORK, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.DONT_REQUEST_MORE_WORK, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.doNotRequestMoreWork = true
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and scheduler_rpc_in_progress is 0 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.SCHEDULER_RPC_IN_PROGRESS, null, null)
        projectsParser.characters("0".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.SCHEDULER_RPC_IN_PROGRESS, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.schedulerRPCInProgress = false
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and scheduler_rpc_in_progress is 1 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.SCHEDULER_RPC_IN_PROGRESS, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.SCHEDULER_RPC_IN_PROGRESS, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.schedulerRPCInProgress = true
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and attached_via_acct_mgr is 0 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.ATTACHED_VIA_ACCT_MGR, null, null)
        projectsParser.characters("0".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.ATTACHED_VIA_ACCT_MGR, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.attachedViaAcctMgr = false
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and attached_via_acct_mgr is 1 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.ATTACHED_VIA_ACCT_MGR, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.ATTACHED_VIA_ACCT_MGR, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.attachedViaAcctMgr = true
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and detach_when_done is 0 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.DETACH_WHEN_DONE, null, null)
        projectsParser.characters("0".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.DETACH_WHEN_DONE, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.detachWhenDone = false
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master url and detach_when_done is 1 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.DETACH_WHEN_DONE, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.DETACH_WHEN_DONE, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.detachWhenDone = true
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and ended is 0 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.ENDED, null, null)
        projectsParser.characters("0".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.ENDED, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.ended = false
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and ended is 1 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.ENDED, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.ENDED, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.ended = true
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and trickle_up_pending is 0 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.TRICKLE_UP_PENDING, null, null)
        projectsParser.characters("0".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.TRICKLE_UP_PENDING, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.trickleUpPending = false
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and trickle_up_pending is 1 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.TRICKLE_UP_PENDING, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.TRICKLE_UP_PENDING, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.trickleUpPending = true
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and project_files_downloaded_time then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(
            null,
            Project.Fields.PROJECT_FILES_DOWNLOADED_TIME,
            null,
            null
        )
        projectsParser.characters("10".toCharArray(), 0, 2)
        projectsParser.endElement(null, Project.Fields.PROJECT_FILES_DOWNLOADED_TIME, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.projectFilesDownloadedTime = 10.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and last_rpc_time then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.LAST_RPC_TIME, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.LAST_RPC_TIME, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.lastRPCTime = 1.0
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and no_cpu_pref is 0 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.NO_CPU_PREF, null, null)
        projectsParser.characters("0".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.NO_CPU_PREF, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.noCPUPref = false
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and no_cpu_pref is 1 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.NO_CPU_PREF, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.NO_CPU_PREF, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.noCPUPref = true
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project Has master_url and no_cuda_pref is 0 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.NO_CUDA_PREF, null, null)
        projectsParser.characters("0".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.NO_CUDA_PREF, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.noCUDAPref = false
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and no_cuda_pref is 1 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.NO_CUDA_PREF, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.NO_CUDA_PREF, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.noCUDAPref = true
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and no_ati_pref is 0 then expect list with matching Project`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.NO_ATI_PREF, null, null)
        projectsParser.characters("0".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.NO_ATI_PREF, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.noATIPref = false
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Project has master_url and no_ati_pref is 1 then expect list with matchingProject`() {
        projectsParser.startElement(null, PROJECT, null, null)
        projectsParser.startElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null, null)
        projectsParser.characters(MASTER_URL.toCharArray(), 0, MASTER_URL.length)
        projectsParser.endElement(null, edu.berkeley.boinc.rpc.MASTER_URL, null)
        projectsParser.startElement(null, Project.Fields.NO_ATI_PREF, null, null)
        projectsParser.characters("1".toCharArray(), 0, 1)
        projectsParser.endElement(null, Project.Fields.NO_ATI_PREF, null)
        projectsParser.endElement(null, PROJECT, null)
        expected.masterURL = MASTER_URL
        expected.noATIPref = true
        Assert.assertEquals(listOf(expected), projectsParser.projects)
    }

    companion object {
        private const val GUI_URL_NAME = "GUI URL Name"
        private const val GUI_URL_DESCRIPTION = "GUI URL Description"
        private const val GUI_URL_URL = "GUI URL URL"
        private const val MASTER_URL = "Master URL"
    }
}
