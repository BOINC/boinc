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
        Assert.assertEquals(3, projectControlsListAdapter.itemCount)
    }

    @Test
    fun `Check content of entries`() {
        Assert.assertEquals("Project 1",
            projectControlsListAdapter.entries[0].data.project?.projectName
        )
        Assert.assertEquals("Project 2",
            projectControlsListAdapter.entries[1].data.project?.projectName
        )
        Assert.assertEquals("Project 3",
            projectControlsListAdapter.entries[2].data.project?.projectName
        )
    }

    @Test(expected = IndexOutOfBoundsException::class)
    fun `Check that getItemCount() fails with IndexOutOfBoundsException when incorrect position provided`() {
        projectControlsListAdapter.run { entries[42] }
    }
}
