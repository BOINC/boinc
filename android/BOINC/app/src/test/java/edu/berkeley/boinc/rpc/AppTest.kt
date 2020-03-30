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
import org.junit.Test
import kotlin.test.junit.JUnitAsserter

class AppTest {
    @Test
    fun `Expect name when user-friendly name is null`() {
        val app = App("Name", null)

        JUnitAsserter.assertEquals("Expected 'Name'", "Name", app.displayName)
    }

    @Test
    fun `Expect name when user-friendly name is empty`() {
        val app = App("Name")

        JUnitAsserter.assertEquals("Expected 'Name'", "Name", app.displayName)
    }

    @Test
    fun `Expect user-friendly name when user-friendly name is not empty`() {
        val app = App("Name", "User-friendly name")

        JUnitAsserter.assertEquals("Expected 'User-friendly name'", "User-friendly name", app.displayName)
    }

    @Test
    fun `Test equals() and hashCode()`() {
        EqualsTester().addEqualityGroup(App("Name"), App("NAME"))
                .addEqualityGroup(App(userFriendlyName = "User-friendly name"),
                        App(userFriendlyName = "USER-FRIENDLY NAME"))
                .testEquals()
    }
}
