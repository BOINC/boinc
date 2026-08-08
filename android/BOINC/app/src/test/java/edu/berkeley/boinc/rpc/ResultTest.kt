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

class ResultTest {
    @Test
    fun `Test equals() and hashCode()`() {
        EqualsTester().addEqualityGroup(Result(), Result())
                .addEqualityGroup(Result("Name"))
                .addEqualityGroup(Result(workUnitName = "Work Unit"))
                .addEqualityGroup(Result(projectURL = "Project URL"))
                .addEqualityGroup(Result(versionNum = 1))
                .addEqualityGroup(Result(planClass = "Plan class"))
                .addEqualityGroup(Result(reportDeadline = 1))
                .addEqualityGroup(Result(receivedTime = 1))
                .addEqualityGroup(Result(finalCPUTime = 1.0))
                .addEqualityGroup(Result(finalElapsedTime = 1.0))
                .addEqualityGroup(Result(state = 1))
                .addEqualityGroup(Result(schedulerState = 1))
                .addEqualityGroup(Result(exitStatus = 1))
                .addEqualityGroup(Result(signal = 1))
                .addEqualityGroup(Result(stderrOut = "Output"))
                .addEqualityGroup(Result(activeTaskState = 1))
                .addEqualityGroup(Result(appVersionNum = 1))
                .addEqualityGroup(Result(slot = 1))
                .addEqualityGroup(Result(pid = 1))
                .addEqualityGroup(Result(checkpointCPUTime = 1.0))
                .addEqualityGroup(Result(currentCPUTime = 1.0))
                .addEqualityGroup(Result(fractionDone = 1f))
                .addEqualityGroup(Result(elapsedTime = 1.0))
                .addEqualityGroup(Result(swapSize = 1.0))
                .addEqualityGroup(Result(workingSetSizeSmoothed = 1.0))
                .testEquals()
    }
}
