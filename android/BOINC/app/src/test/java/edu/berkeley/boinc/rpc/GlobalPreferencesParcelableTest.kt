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

import android.os.Parcel
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class GlobalPreferencesParcelableTest {
    @Test
    fun `Test Creator createFromParcel()`() {
        val expected = GlobalPreferences()
        val parcel = Parcel.obtain()
        expected.writeToParcel(parcel, expected.describeContents())

        // Reset parcel for reading.
        parcel.setDataPosition(0)
        val actual = GlobalPreferences.CREATOR.createFromParcel(parcel)
        Assert.assertEquals(expected, actual)
    }

    @Test
    fun `Test Creator newArray()`() {
        val array = GlobalPreferences.CREATOR.newArray(2)
        Assert.assertNotNull(array)
        Assert.assertEquals(2, array.size)
    }
}
