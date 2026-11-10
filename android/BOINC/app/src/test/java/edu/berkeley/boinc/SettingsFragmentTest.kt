/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2026 University of California
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
package edu.berkeley.boinc

import android.content.SharedPreferences
import edu.berkeley.boinc.client.MonitorAsync
import io.mockk.Called
import io.mockk.every
import io.mockk.mockk
import io.mockk.verify
import org.junit.After
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class SettingsFragmentTest {
    @After
    fun tearDown() {
        BOINCActivity.monitor = null
    }

    // Since SDK 34 the listener contract allows a null key, delivered when
    // shared preferences are cleared; expect it to be a no-op.
    @Test
    fun `Expect no preference reads when onSharedPreferenceChanged() is called with a null key`() {
        BOINCActivity.monitor = mockk<MonitorAsync>(relaxed = true)
        val settingsFragment = SettingsFragment()
        val sharedPreferences = mockk<SharedPreferences>(relaxed = true)

        settingsFragment.onSharedPreferenceChanged(sharedPreferences, null)

        verify { sharedPreferences wasNot Called }
    }

    @Test
    fun `Expect preference value passed to monitor when onSharedPreferenceChanged() is called with autostart key`() {
        val monitor = mockk<MonitorAsync>(relaxed = true)
        BOINCActivity.monitor = monitor
        val settingsFragment = SettingsFragment()
        val sharedPreferences = mockk<SharedPreferences>(relaxed = true)
        every { sharedPreferences.getBoolean("autostart", true) } returns true

        settingsFragment.onSharedPreferenceChanged(sharedPreferences, "autostart")

        verify { monitor.autostart = true }
    }
}
