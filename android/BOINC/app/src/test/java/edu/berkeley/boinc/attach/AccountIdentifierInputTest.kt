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

import android.text.InputType
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.ProjectConfig
import edu.berkeley.boinc.utils.ERR_GETHOSTBYNAME
import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Test

class AccountIdentifierInputTest {
    @Test
    fun `When ProjectConfig is null then expect UNKNOWN`() {
        Assertions.assertEquals(AccountIdentifierInput.UNKNOWN, AccountIdentifierInput.of(null))
    }

    @Test
    fun `When ProjectConfig has error then expect UNKNOWN`() {
        Assertions.assertEquals(AccountIdentifierInput.UNKNOWN,
                AccountIdentifierInput.of(ProjectConfig(errorNum = ERR_GETHOSTBYNAME, usesName = true)))
    }

    @Test
    fun `When ProjectConfig does not use user name then expect EMAIL_ADDRESS`() {
        val input = AccountIdentifierInput.of(ProjectConfig())

        Assertions.assertEquals(AccountIdentifierInput.EMAIL_ADDRESS, input)
        Assertions.assertEquals(R.string.attachproject_login_header_id_email, input.hint)
        Assertions.assertEquals(
                InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS,
                input.inputType)
        Assertions.assertEquals(R.string.attachproject_error_no_email, input.missingError)
    }

    @Test
    fun `When ProjectConfig uses user name then expect USER_NAME`() {
        val input = AccountIdentifierInput.of(ProjectConfig(usesName = true))

        Assertions.assertEquals(AccountIdentifierInput.USER_NAME, input)
        Assertions.assertEquals(R.string.attachproject_acctmgr_header_username, input.hint)
        Assertions.assertEquals(
                InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS,
                input.inputType)
        Assertions.assertEquals(R.string.attachproject_error_no_name, input.missingError)
    }
}
