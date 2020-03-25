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
package edu.berkeley.boinc.rpc;

import android.os.Parcel;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.filters.SmallTest;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import static com.google.common.truth.Truth.assertThat;

@RunWith(AndroidJUnit4.class)
@SmallTest
public class WorkUnitInstrumentedTest {
    private WorkUnit expected;

    @Before
    public void setUp() {
        expected = new WorkUnit();
    }

    @Test
    public void testWorkUnitCreatorCreateFromParcel() {
        final Parcel parcel = Parcel.obtain();
        expected.writeToParcel(parcel, expected.describeContents());

        // Reset parcel for reading.
        parcel.setDataPosition(0);

        final WorkUnit actual = WorkUnit.CREATOR.createFromParcel(parcel);

        assertThat(actual).isEqualTo(expected);
    }

    @Test
    public void testWorkUnitCreatorNewArray() {
        final WorkUnit[] array = WorkUnit.CREATOR.newArray(2);

        assertThat(array).isNotNull();
        assertThat(array.length).isEqualTo(2);
    }
}
