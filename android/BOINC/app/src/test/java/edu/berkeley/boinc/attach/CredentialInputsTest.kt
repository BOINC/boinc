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
package edu.berkeley.boinc.attach

import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.ProjectConfig
import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Test

private val EMAIL_ADDRESS_ONLY = CredentialInputs(emailAddress = true, userName = false)
private val USER_NAME_ONLY = CredentialInputs(emailAddress = false, userName = true)

class CredentialInputsTest {
    @Test
    fun `When no project configs are known then expect all inputs`() {
        Assertions.assertEquals(CredentialInputs.ALL, CredentialInputs.forProjects(emptyList()))
    }

    @Test
    fun `When project uses email address then expect email address only`() {
        Assertions.assertEquals(EMAIL_ADDRESS_ONLY,
                CredentialInputs.forProjects(listOf(ProjectConfig())))
    }

    @Test
    fun `When project uses user name then expect user name for login and both for registration`() {
        val configs = listOf(ProjectConfig(usesName = true))
        Assertions.assertEquals(USER_NAME_ONLY, CredentialInputs.forProjects(configs))
        Assertions.assertEquals(CredentialInputs.ALL, CredentialInputs.forProjects(configs, login = false))
    }

    @Test
    fun `When projects use different identifiers then expect both inputs`() {
        Assertions.assertEquals(CredentialInputs.ALL, CredentialInputs.forProjects(listOf(
                ProjectConfig(),
                ProjectConfig(usesName = true))))
    }

    @Test
    fun `When validating inputs then findError checks only required fields and password length`() {
        Assertions.assertEquals(0, EMAIL_ADDRESS_ONLY.findError("alice@example.com", "", "secret"))
        Assertions.assertEquals(R.string.attachproject_error_no_email, EMAIL_ADDRESS_ONLY.findError("", "alice", "secret"))
        Assertions.assertEquals(R.string.attachproject_error_no_email, EMAIL_ADDRESS_ONLY.findError("   ", "alice", "secret"))

        Assertions.assertEquals(0, USER_NAME_ONLY.findError("", "alice", "secret"))
        Assertions.assertEquals(R.string.attachproject_error_no_name, USER_NAME_ONLY.findError("", "", "secret"))
        Assertions.assertEquals(R.string.attachproject_error_no_name, USER_NAME_ONLY.findError("", "   ", "secret"))
        Assertions.assertEquals(R.string.attachproject_error_no_pwd, USER_NAME_ONLY.findError("", "alice", ""))
        Assertions.assertEquals(R.string.attachproject_error_short_pwd, USER_NAME_ONLY.findError("", "alice", "12345"))

        val minLength8 = CredentialInputs.forProjects(listOf(ProjectConfig(minPwdLength = 8)))
        Assertions.assertEquals(R.string.attachproject_error_short_pwd, minLength8.findError("alice@example.com", "", "1234567"))
        Assertions.assertEquals(0, minLength8.findError("alice@example.com", "", "12345678"))
    }
}
