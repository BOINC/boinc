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
package edu.berkeley.boinc.adapter

import android.graphics.Bitmap
import android.util.Log
import androidx.test.core.app.ApplicationProvider
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.R
import edu.berkeley.boinc.client.MonitorAsync
import edu.berkeley.boinc.rpc.AcctMgrInfo
import edu.berkeley.boinc.rpc.Project
import edu.berkeley.boinc.rpc.ProjectInfo
import edu.berkeley.boinc.utils.TaskRunner
import io.mockk.every
import io.mockk.mockkClass
import io.mockk.mockkStatic
import io.mockk.spyk
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class NavDrawerListAdapterTest {
    private lateinit var navDrawerListAdapter: NavDrawerListAdapter

    @Before
    fun setUp() {
        navDrawerListAdapter = NavDrawerListAdapter(ApplicationProvider.getApplicationContext())
    }

    @Test
    fun `Check that context equals context passed to constructor`() {
        Assert.assertEquals(ApplicationProvider.getApplicationContext(), navDrawerListAdapter.context)
    }

    @Test
    fun `Check that count of items equals 9`() {
        Assert.assertEquals(9, navDrawerListAdapter.count)
    }

    @Test
    fun `Check content and order of items in the list`() {
        val itemTasks = navDrawerListAdapter.getItem(0)
        Assert.assertEquals(R.string.tab_tasks, itemTasks.id)
        Assert.assertEquals(R.drawable.ic_baseline_list, itemTasks.icon)
        Assert.assertTrue(itemTasks.counterVisibility)
        Assert.assertFalse(itemTasks.isSubItem)
        Assert.assertFalse(itemTasks.isProjectItem)

        val itemNotices = navDrawerListAdapter.getItem(1)
        Assert.assertEquals(R.string.tab_notices, itemNotices.id)
        Assert.assertEquals(R.drawable.ic_baseline_email, itemNotices.icon)
        Assert.assertTrue(itemNotices.counterVisibility)
        Assert.assertFalse(itemNotices.isSubItem)
        Assert.assertFalse(itemNotices.isProjectItem)

        val itemProjects = navDrawerListAdapter.getItem(2)
        Assert.assertEquals(R.string.tab_projects, itemProjects.id)
        Assert.assertEquals(R.drawable.ic_projects, itemProjects.icon)
        Assert.assertFalse(itemProjects.counterVisibility)
        Assert.assertFalse(itemProjects.isSubItem)
        Assert.assertFalse(itemProjects.isProjectItem)

        val itemProjectsAdd = navDrawerListAdapter.getItem(3)
        Assert.assertEquals(R.string.projects_add, itemProjectsAdd.id)
        Assert.assertEquals(R.drawable.ic_baseline_add_box, itemProjectsAdd.icon)
        Assert.assertFalse(itemProjectsAdd.counterVisibility)
        Assert.assertTrue(itemProjectsAdd.isSubItem)
        Assert.assertFalse(itemProjectsAdd.isProjectItem)

        val itemPreferences = navDrawerListAdapter.getItem(4)
        Assert.assertEquals(R.string.tab_preferences, itemPreferences.id)
        Assert.assertEquals(R.drawable.ic_baseline_settings, itemPreferences.icon)
        Assert.assertFalse(itemPreferences.counterVisibility)
        Assert.assertFalse(itemPreferences.isSubItem)
        Assert.assertFalse(itemPreferences.isProjectItem)

        val itemHelp = navDrawerListAdapter.getItem(5)
        Assert.assertEquals(R.string.menu_help, itemHelp.id)
        Assert.assertEquals(R.drawable.ic_baseline_help, itemHelp.icon)
        Assert.assertFalse(itemHelp.counterVisibility)
        Assert.assertFalse(itemHelp.isSubItem)
        Assert.assertFalse(itemHelp.isProjectItem)

        val itemReportIssue = navDrawerListAdapter.getItem(6)
        Assert.assertEquals(R.string.menu_report_issue, itemReportIssue.id)
        Assert.assertEquals(R.drawable.ic_baseline_bug_report, itemReportIssue.icon)
        Assert.assertFalse(itemReportIssue.counterVisibility)
        Assert.assertFalse(itemReportIssue.isSubItem)
        Assert.assertFalse(itemReportIssue.isProjectItem)

        val itemAbout = navDrawerListAdapter.getItem(7)
        Assert.assertEquals(R.string.menu_about, itemAbout.id)
        Assert.assertEquals(R.drawable.ic_baseline_info, itemAbout.icon)
        Assert.assertFalse(itemAbout.counterVisibility)
        Assert.assertFalse(itemAbout.isSubItem)
        Assert.assertFalse(itemAbout.isProjectItem)

        val itemEventLog = navDrawerListAdapter.getItem(8)
        Assert.assertEquals(R.string.menu_eventlog, itemEventLog.id)
        Assert.assertEquals(R.drawable.ic_baseline_warning, itemEventLog.icon)
        Assert.assertFalse(itemEventLog.counterVisibility)
        Assert.assertFalse(itemEventLog.isSubItem)
        Assert.assertFalse(itemEventLog.isProjectItem)
    }

    @Test(expected = IndexOutOfBoundsException::class)
    fun `Check that getItem() fails with IndexOutOfBoundsException when incorrect position provided`() {
        navDrawerListAdapter.run { getItem(42) }
    }

    @Test
    fun `Check that getItemId() returns correct Id for the item in the given position`() {
        Assert.assertEquals(R.string.tab_tasks.toLong(), navDrawerListAdapter.getItemId(0))
        Assert.assertEquals(R.string.tab_notices.toLong(), navDrawerListAdapter.getItemId(1))
        Assert.assertEquals(R.string.tab_projects.toLong(), navDrawerListAdapter.getItemId(2))
        Assert.assertEquals(R.string.projects_add.toLong(), navDrawerListAdapter.getItemId(3))
        Assert.assertEquals(R.string.tab_preferences.toLong(), navDrawerListAdapter.getItemId(4))
        Assert.assertEquals(R.string.menu_help.toLong(), navDrawerListAdapter.getItemId(5))
        Assert.assertEquals(R.string.menu_report_issue.toLong(), navDrawerListAdapter.getItemId(6))
        Assert.assertEquals(R.string.menu_about.toLong(), navDrawerListAdapter.getItemId(7))
        Assert.assertEquals(R.string.menu_eventlog.toLong(), navDrawerListAdapter.getItemId(8))
    }

    @Test(expected = IndexOutOfBoundsException::class)
    fun `Check that getItemId() fails with IndexOutOfBoundsException when incorrect position provided`() {
        navDrawerListAdapter.run { getItemId(42) }
    }

    @Test
    fun `Check that getItemForId() returns correct item for the given Id`() {
        Assert.assertEquals(R.string.tab_tasks, navDrawerListAdapter.getItemForId(R.string.tab_tasks)?.id)
        Assert.assertEquals(R.string.tab_notices, navDrawerListAdapter.getItemForId(R.string.tab_notices)?.id)
        Assert.assertEquals(R.string.tab_projects, navDrawerListAdapter.getItemForId(R.string.tab_projects)?.id)
        Assert.assertEquals(R.string.projects_add, navDrawerListAdapter.getItemForId(R.string.projects_add)?.id)
        Assert.assertEquals(R.string.tab_preferences, navDrawerListAdapter.getItemForId(R.string.tab_preferences)?.id)
        Assert.assertEquals(R.string.menu_help, navDrawerListAdapter.getItemForId(R.string.menu_help)?.id)
        Assert.assertEquals(R.string.menu_report_issue, navDrawerListAdapter.getItemForId(R.string.menu_report_issue)?.id)
        Assert.assertEquals(R.string.menu_about, navDrawerListAdapter.getItemForId(R.string.menu_about)?.id)
        Assert.assertEquals(R.string.menu_eventlog, navDrawerListAdapter.getItemForId(R.string.menu_eventlog)?.id)
    }

    @Test
    fun `Check that getItemForId() returns null when incorrect Id provided`() {
        Assert.assertNull(navDrawerListAdapter.getItemForId(R.string.about_title))
    }

    @Test
    fun `Check that getProjectIconForMasterUrl() returns correct Bitmap`() {
        val icon = Bitmap.createBitmap(10, 10, Bitmap.Config.ARGB_8888)

        val monitor = mockkClass(MonitorAsync::class)
        BOINCActivity.monitor = monitor
        every { monitor.getProjectIcon(any()) } returns icon

        Assert.assertEquals(icon, navDrawerListAdapter.getProjectIconForMasterUrl("https://test.com/boinc"))
    }

    @Test
    fun `Check that getProjectIconForMasterUrl() return null when monitor equals null`() {
        BOINCActivity.monitor = null
        Assert.assertNull(navDrawerListAdapter.getProjectIconForMasterUrl("https://test.com/boinc"))
    }

    @Test
    fun `Check that getProjectIconForMasterUrl() returns null when passed url is null`() {
        BOINCActivity.monitor = null
        Assert.assertNull(navDrawerListAdapter.getProjectIconForMasterUrl(null))

        val monitor = mockkClass(MonitorAsync::class)
        BOINCActivity.monitor = monitor
        Assert.assertNull(navDrawerListAdapter.getProjectIconForMasterUrl(null))
    }

    @Test
    fun `Check that getProjectIconForMasterUrl() returns null when exception happens`() {
        mockkStatic(Log::class)

        val monitor = mockkClass(MonitorAsync::class)
        BOINCActivity.monitor = monitor
        every { monitor.getProjectIcon(any()) } throws(Exception())

        Assert.assertNull(navDrawerListAdapter.getProjectIconForMasterUrl("https://test.com/boinc"))
    }

    @Test
    fun `Check that getProjectNameForMasterUrl() returns empty string when monitor equals null`() {
        BOINCActivity.monitor = null
        Assert.assertEquals("", navDrawerListAdapter.getProjectNameForMasterUrl("https://test.com/boinc"))
    }

    @Test
    fun `Check that getProjectNameForMasterUrl() returns empty string when passed url is null`() {
        BOINCActivity.monitor = null
        Assert.assertEquals("", navDrawerListAdapter.getProjectNameForMasterUrl(null))

        val monitor = mockkClass(MonitorAsync::class)
        BOINCActivity.monitor = monitor
        Assert.assertEquals("", navDrawerListAdapter.getProjectNameForMasterUrl(null))
    }

    @Test
    fun `Check that getProjectNameForMasterUrl() returns empty string when exception happens`() {
        mockkStatic(Log::class)

        val monitor = mockkClass(MonitorAsync::class)
        BOINCActivity.monitor = monitor
        every { monitor.getProjectIcon(any()) } throws(Exception())

        Assert.assertEquals("", navDrawerListAdapter.getProjectNameForMasterUrl("https://test.com/boinc"))
    }

    @Test
    fun `Check that getProjectNameForMasterUrl() returns correct Name`() {
        val pi = ProjectInfo("Test Project")
        val monitor = mockkClass(MonitorAsync::class)
        BOINCActivity.monitor = monitor
        every { monitor.getProjectInfo(any()) } returns pi
        every { monitor.getProjectInfoAsync(any(), any()) } returns TaskRunner(null, { monitor.getProjectInfo("Test Project") })

        Assert.assertEquals("Test Project", navDrawerListAdapter.getProjectNameForMasterUrl("https://test.com/boinc"))
    }

    @Test
    fun `Check that getProjectNameForMasterUrl() returns empty string when ProjectInfo is null`() {
        val monitor = mockkClass(MonitorAsync::class)
        BOINCActivity.monitor = monitor
        every { monitor.getProjectInfo(any()) } returns null
        every { monitor.getProjectInfoAsync(any(), any()) } returns TaskRunner(null, { monitor.getProjectInfo("Test Project") })

        Assert.assertEquals("", navDrawerListAdapter.getProjectNameForMasterUrl("https://test.com/boinc"))
    }

    @Test
    fun `Check that compareAndAddProjects() preserve all non-project items when empty list passed`() {
        val expectedList = mutableListOf<NavDrawerListAdapter.NavDrawerItem>()

        for (i in 0 until navDrawerListAdapter.count) {
            expectedList.add(navDrawerListAdapter.getItem(i))
        }
        Assert.assertEquals(navDrawerListAdapter.count, expectedList.size)

        val itemsAdded = navDrawerListAdapter.compareAndAddProjects(listOf())
        Assert.assertEquals(0, itemsAdded)
        Assert.assertEquals(expectedList.size, navDrawerListAdapter.count)

        for (i in 0 until expectedList.size) {
            Assert.assertEquals(expectedList[i], navDrawerListAdapter.getItem(i))
        }
    }

    @Test
    fun `Check that compareAndAddProjects() removes existing project items and adds new from the passed list`() {
        val project = Project("https://test.com/boinc", "", 0f, "Test Project")
        val originalItemsCount = navDrawerListAdapter.count

        var itemsAdded = navDrawerListAdapter.compareAndAddProjects(listOf(project))
        Assert.assertEquals(1, itemsAdded)
        Assert.assertEquals(originalItemsCount + 1, navDrawerListAdapter.count)

        var projectItemsFound = 0
        for (i in 0 until navDrawerListAdapter.count) {
            val item = navDrawerListAdapter.getItem(i)
            if (item.isProjectItem) {
                projectItemsFound++
                Assert.assertEquals(project.projectName, item.title)
                Assert.assertEquals(project.masterURL, item.projectMasterUrl)
            }
        }
        Assert.assertEquals(1, projectItemsFound)

        val projects = listOf(
            Project("https://test.org", "", 0f, "Test Org"),
            Project("https://test.gg", "", 0f, "Test GG")
        )

        itemsAdded = navDrawerListAdapter.compareAndAddProjects(projects)
        Assert.assertEquals(2, itemsAdded)
        Assert.assertEquals(originalItemsCount + 2, navDrawerListAdapter.count)

        projectItemsFound = 0
        var firstProjectFound = false
        var secondProjectFound = false
        for(i in 0 until navDrawerListAdapter.count) {
            val item = navDrawerListAdapter.getItem(i)
            if (item.isProjectItem) {
                projectItemsFound++
                if (item.projectMasterUrl == "https://test.org" && !firstProjectFound) {
                    firstProjectFound = true
                    Assert.assertEquals("Test Org", item.title)
                } else if (item.projectMasterUrl == "https://test.gg" && !secondProjectFound) {
                    secondProjectFound = true
                    Assert.assertEquals("Test GG", item.title)
                } else {
                    // Should not be reachable
                    Assert.assertFalse(true)
                }
            }
        }
        Assert.assertTrue(firstProjectFound)
        Assert.assertTrue(secondProjectFound)
        Assert.assertEquals(2, projectItemsFound)
    }

    @Test
    fun `Check that compareAndAddProjects() does not change items when null passed as a project list`() {
        val itemsAdded = navDrawerListAdapter.compareAndAddProjects(null)
        Assert.assertEquals(0, itemsAdded)
    }

    @Test
    fun `Check that isAccountManagerPresent() returns correct status`() {
        val accountManagerPresent = AcctMgrInfo("Test", "https://test.com",
            isHavingCredentials = false,
            isPresent = true
        )
        val accountManagerNotPresent = AcctMgrInfo(
            "Test", "https://test.com",
            isHavingCredentials = false,
            isPresent = false
        )

        val monitor = mockkClass(MonitorAsync::class)
        BOINCActivity.monitor = monitor

        every { monitor.clientAcctMgrInfo } returns accountManagerPresent
        Assert.assertTrue(navDrawerListAdapter.isAccountManagerPresent)

        every { monitor.clientAcctMgrInfo } returns accountManagerNotPresent
        Assert.assertFalse(navDrawerListAdapter.isAccountManagerPresent)
    }

    @Test
    fun `Check that isAccountManagerPresent() returns False when Monitor equals Null`() {
        BOINCActivity.monitor = null
        Assert.assertFalse(navDrawerListAdapter.isAccountManagerPresent)
    }

    @Test
    fun `Check that isAccountManagerPresent() returns False when exception happens`() {
        val monitor = mockkClass(MonitorAsync::class)
        BOINCActivity.monitor = monitor

        every { monitor.clientAcctMgrInfo } throws(Exception())
        Assert.assertFalse(navDrawerListAdapter.isAccountManagerPresent)
    }

    @Test
    fun `Check that updateUseAccountManagerItem() has correct visibility of 'Use Account Manager' when Account Manager has different states`() {
        spyk(navDrawerListAdapter)
        every { navDrawerListAdapter.isAccountManagerPresent } returns false

        Assert.assertNull(navDrawerListAdapter.getItemForId(R.string.attachproject_acctmgr_header))

        navDrawerListAdapter.updateUseAccountManagerItem()
        Assert.assertNotNull(navDrawerListAdapter.getItemForId(R.string.attachproject_acctmgr_header))

        // This is not duplicate but verification that item is kept on update
        navDrawerListAdapter.updateUseAccountManagerItem()
        Assert.assertNotNull(navDrawerListAdapter.getItemForId(R.string.attachproject_acctmgr_header))

        every { navDrawerListAdapter.isAccountManagerPresent } returns true

        navDrawerListAdapter.updateUseAccountManagerItem()
        Assert.assertNull(navDrawerListAdapter.getItemForId(R.string.attachproject_acctmgr_header))

        // This is not duplicate but verification that item is not added back on update
        navDrawerListAdapter.updateUseAccountManagerItem()
        Assert.assertNull(navDrawerListAdapter.getItemForId(R.string.attachproject_acctmgr_header))
    }
}
