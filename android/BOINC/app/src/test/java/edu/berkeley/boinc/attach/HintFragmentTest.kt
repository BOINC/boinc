/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2022 University of California
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
package edu.berkeley.boinc.attach

import android.view.LayoutInflater
import androidx.fragment.app.testing.launchFragmentInContainer
import org.hamcrest.MatcherAssert.assertThat
import org.hamcrest.CoreMatchers.instanceOf
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class HintFragmentTest {
    private lateinit var fragment: HintFragment
    private lateinit var inflater: LayoutInflater

    @Before
    fun setUp() {
        val scenario = launchFragmentInContainer<HintFragment>()
        scenario.onFragment {
            inflater = it.layoutInflater
        }
    }

    @Test
    fun `Check that onCreate returns not null`() {
        fragment = HintFragment.newInstance(1)
        Assert.assertNotNull(fragment.onCreateView(inflater, null, null))
        fragment = HintFragment.newInstance(2)
        Assert.assertNotNull(fragment.onCreateView(inflater, null, null))
        fragment = HintFragment.newInstance(3)
        Assert.assertNotNull(fragment.onCreateView(inflater, null, null))
    }

    @Test
    fun `Check that onCreate returns null when unspecified type passed`() {
        fragment = HintFragment.newInstance(4)
        Assert.assertNull(fragment.onCreateView(inflater, null, null))
    }

    @Test
    fun `Check that newInstance() Returns HintFragment Instance`() {
        assertThat(HintFragment.newInstance(2), instanceOf(HintFragment::class.java))
    }
}
