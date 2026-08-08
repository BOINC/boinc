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

import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Test

class AttachActivityItemTest {
    @Test
    fun `Check that AttachActivityItem is saved when passed to constructor`() {
        val type = AttachActivityItemType.ALL_PROJECTS
        val text = "Test Project Name"
        val description = "Test Project Description"

        val item = AttachActivityItem(type, text, description)

        Assertions.assertEquals(type, item.type)
        Assertions.assertEquals(text, item.text)
        Assertions.assertEquals(description, item.description)
    }

    @Test
    fun `Check that AttachActivityItemType contains only 'ALL_PROJECTS' and 'ACCOUNT_MANAGER' types`() {
        Assertions.assertEquals(2, AttachActivityItemType.values().size)
        Assertions.assertEquals("ALL_PROJECTS", AttachActivityItemType.ALL_PROJECTS.name)
        Assertions.assertEquals("ACCOUNT_MANAGER", AttachActivityItemType.ACCOUNT_MANAGER.name)
    }
}
