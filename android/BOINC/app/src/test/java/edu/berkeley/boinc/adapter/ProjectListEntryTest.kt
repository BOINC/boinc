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

import edu.berkeley.boinc.rpc.ProjectInfo
import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Test

class ProjectListEntryTest {
    @Test
    fun `Check that ProjectInfo is saved when passed to constructor`()
    {
        val projectInfo = ProjectInfo("ProjectName", "https://testproject.com",
                                      "Mathematics", "Combinatorics",
                                      "Test Project Description",
                                      "Home Sweet Home", listOf("windows"),
                                      "https://testproject.com/favicon.png",
                                      "Test Summary of the Project")
        val projectListEntry = ProjectListEntry(projectInfo)

        Assertions.assertEquals(projectInfo, projectListEntry.info)
    }

    @Test
    fun `Check that isChecked is set to False when ProjectInfo is passed to constructor`() {
        val projectInfo = ProjectInfo("Test Project", "https://testproject.com")
        val projectListEntry = ProjectListEntry(projectInfo)

        Assertions.assertFalse(projectListEntry.isChecked)
    }

    @Test
    fun `Check that isChecked correctly saves its state`() {
        val projectInfo = ProjectInfo("Test Project", "https://testproject.com")
        val projectListEntry = ProjectListEntry(projectInfo)

        Assertions.assertFalse(projectListEntry.isChecked)

        projectListEntry.isChecked = true
        Assertions.assertTrue(projectListEntry.isChecked)

        projectListEntry.isChecked = false
        Assertions.assertFalse(projectListEntry.isChecked)
    }
}
