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
import org.apache.commons.lang3.SerializationUtils
import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Test

class CcStatusTest {
    @Test
    fun `Test equals() and hashCode()`() {
        EqualsTester().addEqualityGroup(CcStatus(), CcStatus())
                .addEqualityGroup(CcStatus(taskMode = 1))
                .addEqualityGroup(CcStatus(taskModePerm = 1))
                .addEqualityGroup(CcStatus(taskModeDelay = 1.0))
                .addEqualityGroup(CcStatus(taskSuspendReason = 1))
                .addEqualityGroup(CcStatus(networkMode = 1))
                .addEqualityGroup(CcStatus(networkModePerm = 1))
                .addEqualityGroup(CcStatus(networkModeDelay = 1.0))
                .addEqualityGroup(CcStatus(networkSuspendReason = 1))
                .addEqualityGroup(CcStatus(networkStatus = 1))
                .addEqualityGroup(CcStatus(amsPasswordError = true))
                .addEqualityGroup(CcStatus(managerMustQuit = true))
                .addEqualityGroup(CcStatus(disallowAttach = true))
                .addEqualityGroup(CcStatus(simpleGuiOnly = true))
                .testEquals()
    }

    @Test
    fun `Test serialization`() {
        val original = CcStatus()
        val copy = SerializationUtils.clone(original)

        Assertions.assertEquals(original, copy)
    }
}
