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

import com.google.common.testing.EqualsTester
import org.junit.jupiter.api.Test

class NoticeTest {
    @Test
    fun `Test equals() and hashCode()`() {
        EqualsTester().addEqualityGroup(Notice(), Notice())
                .addEqualityGroup(Notice(1))
                .addEqualityGroup(Notice(title = "Title"))
                .addEqualityGroup(Notice(description = "Description"))
                .addEqualityGroup(Notice(createTime = 1.0))
                .addEqualityGroup(Notice(arrivalTime = 1.0))
                .addEqualityGroup(Notice(isPrivate = true))
                .addEqualityGroup(Notice(category = "Category"))
                .addEqualityGroup(Notice(link = "Link"))
                .addEqualityGroup(Notice(projectName = "Project"))
                .addEqualityGroup(Notice(isServerNotice = true))
                .addEqualityGroup(Notice(isClientNotice = true))
                .testEquals()
    }
}
