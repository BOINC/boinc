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
import org.junit.Before
import org.junit.Test
import kotlin.test.junit.JUnitAsserter

class ProjectConfigTest {
    private lateinit var projectConfig: ProjectConfig

    @Before
    fun setUp() {
        projectConfig = ProjectConfig(masterUrl = MASTER_URL)
    }

    @Test
    fun `Expect master URL when RPC URL base is empty`() {
        JUnitAsserter.assertEquals("Expected $MASTER_URL", MASTER_URL, projectConfig.secureUrlIfAvailable)
    }

    @Test
    fun `Expect RPC URL base when RPC URL base is not empty`() {
        projectConfig.webRpcUrlBase = RPC_URL_BASE
        JUnitAsserter.assertEquals("Expected $RPC_URL_BASE", RPC_URL_BASE, projectConfig.secureUrlIfAvailable)
    }

    @Test
    fun `Test equals() and hashCode()`() {
        projectConfig.masterUrl = ""
        EqualsTester().addEqualityGroup(this.projectConfig, ProjectConfig())
                .addEqualityGroup(ProjectConfig(1))
                .addEqualityGroup(ProjectConfig(name = "Name"))
                .addEqualityGroup(ProjectConfig(masterUrl = MASTER_URL))
                .addEqualityGroup(ProjectConfig(webRpcUrlBase = RPC_URL_BASE))
                .addEqualityGroup(ProjectConfig(localRevision = "Revision"))
                .addEqualityGroup(ProjectConfig(minPwdLength = 1))
                .addEqualityGroup(ProjectConfig(minClientVersion = 1))
                .addEqualityGroup(ProjectConfig(rpcPrefix = "RPC prefix"))
                .addEqualityGroup(ProjectConfig(platforms = mutableListOf(PlatformInfo())))
                .addEqualityGroup(ProjectConfig(termsOfUse = "Terms of use"))
                .addEqualityGroup(ProjectConfig(usesName = true))
                .addEqualityGroup(ProjectConfig(webStopped = true))
                .addEqualityGroup(ProjectConfig(schedulerStopped = true))
                .addEqualityGroup( ProjectConfig(accountCreationDisabled = true))
                .addEqualityGroup(ProjectConfig(clientAccountCreationDisabled = true))
                .addEqualityGroup(ProjectConfig(accountManager = true))
                .testEquals()
    }

    companion object {
        private const val MASTER_URL = "Master URL"
        private const val RPC_URL_BASE = "RPC URL Base"
    }
}
