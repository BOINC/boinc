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
package edu.berkeley.boinc.rpc

import org.junit.Before
import org.junit.Test
import kotlin.test.junit.JUnitAsserter

class ProjectTest {
    private lateinit var project: Project

    @Before
    fun setUp() {
        project = Project(masterURL = "Master URL")
    }

    @Test
    fun `Expect getName() to return master URL when project name is empty`() {
        JUnitAsserter.assertEquals("Expected to be equal.","Master URL", project.name)
    }

    @Test
    fun `Expect getName() to return project name when project name is not empty`() {
        project.projectName = "Project"
        JUnitAsserter.assertEquals("Expected to be equal.","Project", project.name)
    }
}
