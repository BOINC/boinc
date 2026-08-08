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
import org.junit.jupiter.api.BeforeAll
import org.junit.jupiter.api.Test
import org.junit.jupiter.api.TestInstance

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
class GuiLogRecyclerViewAdapterTest {
    private lateinit var adapter: GuiLogRecyclerViewAdapter
    private lateinit var messages: List<String>

    @BeforeAll
    fun setUp() {
        messages = listOf(
                "Test log message 0",
                "Test log message 1",
                "Test log message 2",
                "Test log message 3",
                "Test log message 4"
        )
        adapter = GuiLogRecyclerViewAdapter(messages)
    }

    @Test
    fun `Expect getItemCount() equal 5`() {
        Assertions.assertEquals(5, adapter.itemCount)
    }
}
