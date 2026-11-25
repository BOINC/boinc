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
import androidx.annotation.StringRes
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.ProjectConfig
import edu.berkeley.boinc.utils.ERR_OK

/**
 * Presentation of the account identifier input of an account manager, which identifies accounts
 * either by email address or by user name (see [ProjectConfig.usesName]).
 */
enum class AccountIdentifierInput(
    @get:StringRes val hint: Int,
    val inputType: Int,
    @get:StringRes val missingError: Int
) {
    EMAIL_ADDRESS(
        R.string.attachproject_login_header_id_email,
        InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS,
        R.string.attachproject_error_no_email
    ),
    USER_NAME(
        R.string.attachproject_acctmgr_header_username,
        InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS,
        R.string.attachproject_error_no_name
    ),

    // used as long as the configuration of the account manager is not known
    UNKNOWN(
        R.string.attachproject_acctmgr_header_name,
        InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS,
        R.string.attachproject_error_no_name
    );

    companion object {
        /**
         * Returns the input matching [config], or [UNKNOWN] if no configuration could be retrieved.
         */
        fun of(config: ProjectConfig?) = when {
            config == null || config.errorNum != ERR_OK -> UNKNOWN
            config.usesName -> USER_NAME
            else -> EMAIL_ADDRESS
        }
    }
}
