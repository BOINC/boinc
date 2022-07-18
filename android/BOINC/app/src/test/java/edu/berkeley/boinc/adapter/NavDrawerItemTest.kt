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

import android.content.Context
import android.graphics.Bitmap
import androidx.test.core.app.ApplicationProvider
import edu.berkeley.boinc.R
import io.mockk.every
import io.mockk.spyk
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class NavDrawerItemTest {
    private lateinit var context : Context
    private lateinit var adapter : NavDrawerListAdapter

    @Before
    fun setUp() {
        context = ApplicationProvider.getApplicationContext()
        adapter = spyk(NavDrawerListAdapter(context))
    }

    @Test
    fun `Check default item values when constructor is called with (id, icon)`() {
        val actual = adapter.NavDrawerItem(R.string.tab_projects, 42)

        Assert.assertEquals(R.string.tab_projects, actual.id)
        Assert.assertEquals(context.getString(R.string.tab_projects), actual.title)
        Assert.assertEquals(42, actual.icon)

        Assert.assertFalse(actual.counterVisibility)
        Assert.assertFalse(actual.isSubItem)
        Assert.assertFalse(actual.isProjectItem)
        Assert.assertNull(actual.projectIcon)
        Assert.assertNull(actual.projectMasterUrl)
    }

    @Test
    fun `Check default item values when constructor is called with (id, icon, isCounterVisible, isSubItem)` () {
        val actual = adapter.NavDrawerItem(R.string.tab_projects, 42,
            isCounterVisible = true,
            isSubItem = true
        )

        Assert.assertEquals(R.string.tab_projects, actual.id)
        Assert.assertEquals(context.getString(R.string.tab_projects), actual.title)
        Assert.assertEquals(42, actual.icon)

        Assert.assertTrue(actual.counterVisibility)
        Assert.assertTrue(actual.isSubItem)
        Assert.assertFalse(actual.isProjectItem)
        Assert.assertNull(actual.projectIcon)
        Assert.assertNull(actual.projectMasterUrl)
    }

    @Test
    fun `Check default item values when constructor is called with (name, icon, masterUrl)` () {
        val icon = Bitmap.createBitmap(10, 10, Bitmap.Config.ARGB_8888)
        val actual = adapter.NavDrawerItem("Test Project", icon, "https://test.com/boinc")

        Assert.assertEquals("https://test.com/boinc".hashCode(), actual.id)
        Assert.assertEquals("Test Project", actual.title)
        Assert.assertEquals(0, actual.icon)

        Assert.assertFalse(actual.counterVisibility)
        Assert.assertTrue(actual.isSubItem)
        Assert.assertTrue(actual.isProjectItem)
        Assert.assertEquals(icon, actual.projectIcon)
        Assert.assertEquals("https://test.com/boinc", actual.projectMasterUrl)
    }

    @Test
    fun `Check default item values when constructor is called with (id, icon, isCounterVisible)` () {
        val actual = adapter.NavDrawerItem(R.string.tab_projects, 42, true)

        Assert.assertEquals(R.string.tab_projects, actual.id)
        Assert.assertEquals(context.getString(R.string.tab_projects), actual.title)
        Assert.assertEquals(42, actual.icon)

        Assert.assertTrue(actual.counterVisibility)
        Assert.assertFalse(actual.isSubItem)
        Assert.assertFalse(actual.isProjectItem)
        Assert.assertNull(actual.projectIcon)
        Assert.assertNull(actual.projectMasterUrl)
    }

    @Test
    fun `Check that updateProjectIcon() updates project icon`() {
        val icon = Bitmap.createBitmap(10, 10, Bitmap.Config.ARGB_8888)

        val actual = adapter.NavDrawerItem("Test Project", null, "https://test.com/boinc")

        Assert.assertNull(actual.projectIcon)

        every { adapter.getProjectIconForMasterUrl(any()) } returns icon

        actual.updateProjectIcon()
        Assert.assertEquals(icon, actual.projectIcon)
    }

    @Test
    fun `Check that updateProjectName() updates project name`() {
        val actual = adapter.NavDrawerItem("Test Project", null, "https://test.com/boinc")

        Assert.assertEquals("Test Project", actual.title)

        every { adapter.getProjectNameForMasterUrl(any()) } returns "New Test Project"

        actual.updateProjectName()
        Assert.assertEquals("New Test Project", actual.title)
    }

    @Test
    fun `Check that setTitle() sets project name`() {
        val actual = adapter.NavDrawerItem("Test Project", null, "https://test.com/boinc")

        Assert.assertEquals("Test Project", actual.title)

        actual.title = "New Test Project"
        Assert.assertEquals("New Test Project", actual.title)
    }

    @Test
    fun `Check that setIcon() updates project icon`() {
        val actual = adapter.NavDrawerItem("Test Project", null, "https://test.com/boinc")

        Assert.assertEquals(0, actual.icon)

        actual.icon = 42
        Assert.assertEquals(42, actual.icon)
    }
}
