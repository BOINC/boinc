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

import androidx.test.core.app.ApplicationProvider
import edu.berkeley.boinc.ProjectsFragment
import edu.berkeley.boinc.rpc.Project
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class ProjectControlsListAdapterTest {
    private lateinit var projectControlsListAdapter: ProjectControlsListAdapter

    @Before
    fun setUp() {
        val projectsFragment = ProjectsFragment()
        val projectControls = listOf(
            projectsFragment.ProjectControl(
                projectsFragment.ProjectsListData(
                    Project(projectName = "Project 1"),
                    null,
                    null
                ),
                0
            ),
            projectsFragment.ProjectControl(
                projectsFragment.ProjectsListData(
                    Project(projectName = "Project 2"),
                    null,
                    null
                ),
                1
            ),
            projectsFragment.ProjectControl(
                projectsFragment.ProjectsListData(
                    Project(projectName = "Project 3"),
                    null,
                    null
                ),
                2
            )
        )
        projectControlsListAdapter = ProjectControlsListAdapter(
            ApplicationProvider.getApplicationContext(),
            projectControls
        )
    }

    @Test
    fun `Check that entry count equal passed count`() {
        Assert.assertEquals(3, projectControlsListAdapter.count)
    }

    @Test
    fun `Check content and order of entries`() {
        Assert.assertEquals("Project 1",
            projectControlsListAdapter.getItem(0).data.project?.projectName
        )
        Assert.assertEquals(0, projectControlsListAdapter.getItemId(0))
        Assert.assertEquals("Project 2",
            projectControlsListAdapter.getItem(1).data.project?.projectName
        )
        Assert.assertEquals(1, projectControlsListAdapter.getItemId(1))
        Assert.assertEquals("Project 3",
            projectControlsListAdapter.getItem(2).data.project?.projectName
        )
        Assert.assertEquals(2, projectControlsListAdapter.getItemId(2))
    }

    @Test(expected = IndexOutOfBoundsException::class)
    fun `Check that getItem() fails with IndexOutOfBoundsException when incorrect position provided`() {
        projectControlsListAdapter.run { getItem(42) }
    }

    @Test(expected = IndexOutOfBoundsException::class)
    fun `Check that getItemId() fails with IndexOutOfBoundsException when incorrect position provided`() {
        projectControlsListAdapter.run { getItemId(42) }
    }
}
