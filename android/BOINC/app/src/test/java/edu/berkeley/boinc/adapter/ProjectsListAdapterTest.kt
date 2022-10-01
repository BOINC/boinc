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
package edu.berkeley.boinc.adapter

import android.view.ViewGroup
import androidx.fragment.app.FragmentActivity
import androidx.fragment.app.testing.launchFragmentInContainer
import edu.berkeley.boinc.ProjectsFragment
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.Project
import org.apache.commons.lang3.builder.EqualsBuilder
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.powermock.api.mockito.PowerMockito.mock
import org.robolectric.RobolectricTestRunner


@RunWith(RobolectricTestRunner::class)
class ProjectsListAdapterTest {
    private lateinit var projectsListAdapter: ProjectsListAdapter
    private lateinit var fragActivity: FragmentActivity
    private lateinit var projectsList: List<ProjectsFragment.ProjectsListData>
    private lateinit var viewGroup: ViewGroup

    @Before
    fun setUp() {
        val projectsFragment = ProjectsFragment()
        viewGroup = mock(ViewGroup::class.java)
        val scenario = launchFragmentInContainer<ProjectsFragment>()
        scenario.onFragment {
            fragActivity = it.activity!!
        }
        projectsList = listOf(
            projectsFragment.ProjectsListData(
                Project(projectName = "Project 1", diskUsage = 5000000.234),
                null,
                listOf()
            ),
            projectsFragment.ProjectsListData(
                Project(projectName = "Project 2", diskUsage = 5000000.345),
                null,
                listOf()
            ),
            projectsFragment.ProjectsListData(
                Project(projectName = "Project 3", diskUsage = 5000000.456),
                null,
                listOf()
            )
        )
        projectsListAdapter = ProjectsListAdapter(
            fragActivity,
            fragActivity.findViewById(R.id.projects_list),
            R.id.projects_list,
            projectsList
        )
    }

    @Test
    fun `Check that entry count equal passed count`() {
        Assert.assertEquals(3, projectsListAdapter.count)
    }

    @Test
    fun getItemTest() {
        Assert.assertTrue(
            EqualsBuilder.reflectionEquals(
                projectsList[1],
                projectsListAdapter.getItem(1)
            )
        )
    }

    @Test
    fun getItemIdTest() {
        Assert.assertEquals(2L, projectsListAdapter.getItemId(2))
    }

    @Test
    fun getDiskUsageTest() {
        Assert.assertEquals("4.77", projectsListAdapter.getDiskUsage(1))
    }

    @Test
    fun getNameTest() {
        Assert.assertEquals("Project 2", projectsListAdapter.getName(1))
    }

    @Test
    fun getViewNotNullTest() {
        Assert.assertNotNull(projectsListAdapter.getView(0, null, viewGroup))
    }

    @Test
    fun `Check content and order of entries`() {
        Assert.assertEquals(
            "Project 1", projectsListAdapter.getItem(0).project?.projectName
        )
        Assert.assertEquals(0, projectsListAdapter.getItemId(0))
        Assert.assertEquals(
            "Project 2", projectsListAdapter.getItem(1).project?.projectName
        )
        Assert.assertEquals(1, projectsListAdapter.getItemId(1))
        Assert.assertEquals(
            "Project 3", projectsListAdapter.getItem(2).project?.projectName
        )
        Assert.assertEquals(2, projectsListAdapter.getItemId(2))
    }
}
