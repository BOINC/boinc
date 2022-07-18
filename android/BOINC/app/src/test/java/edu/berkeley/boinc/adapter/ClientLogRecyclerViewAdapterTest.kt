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

import edu.berkeley.boinc.rpc.Message
import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.BeforeAll
import org.junit.jupiter.api.Test
import org.junit.jupiter.api.TestInstance
import java.time.Instant
import java.time.LocalDateTime
import java.time.ZoneId
import java.time.ZoneOffset
import java.time.format.DateTimeFormatter
import java.time.format.FormatStyle

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
class ClientLogRecyclerViewAdapterTest {
    private lateinit var adapter: ClientLogRecyclerViewAdapter
    private lateinit var messages: List<Message>

    @BeforeAll
    fun setUp() {
        val instant: LocalDateTime = LocalDateTime.now()
        val systemZone = ZoneId.systemDefault()
        val zoneOffset: ZoneOffset = systemZone.rules.getOffset(instant)
        messages = listOf(
                Message("Project 1", 0, 0,
                        LocalDateTime.now().toEpochSecond(zoneOffset),
                        "Test text for Project 1 part 1"),
                Message("Project 1", 0, 1,
                        LocalDateTime.now().toEpochSecond(zoneOffset),
                        "Test text for Project 1 part 2"),
                Message("Project 2", 1, 0,
                        LocalDateTime.now().toEpochSecond(zoneOffset),
                        "Test text for Project 2"),
                Message("", 0, 0,
                        LocalDateTime.now().toEpochSecond(zoneOffset),
                        "Test text for no project"),
                Message("Project 1", 1, 2,
                        LocalDateTime.now().toEpochSecond(zoneOffset),
                        "Test text for Project 1 part 3")
        )
        adapter = ClientLogRecyclerViewAdapter(messages)
    }

    @Test
    fun `Expect getItemCount() equal 5`() {
        Assertions.assertEquals(5, adapter.itemCount)
    }

    @Test
    fun `Expect correct result of getProject()`() {
        for (i in messages.indices) {
            Assertions.assertEquals(messages[i].project, adapter.getProject(i))
        }
    }

    @Test
    fun `Except correct result of getMessage()`() {
        for (i in messages.indices) {
            Assertions.assertEquals(messages[i].body, adapter.getMessage(i))
        }
    }

    @Test
    fun `Expect correct result of getDateTimeString()`() {
        val dtf = DateTimeFormatter.ofLocalizedDateTime(FormatStyle.MEDIUM)
        for (i in messages.indices) {
            Assertions.assertEquals(dtf.format(LocalDateTime.ofInstant(Instant.ofEpochSecond(
                    messages[i].timestamp), ZoneId.systemDefault())),
                    adapter.getDateTimeString(i))
        }
    }
}
