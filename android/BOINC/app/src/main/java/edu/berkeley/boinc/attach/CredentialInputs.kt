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

import androidx.annotation.StringRes
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.ProjectConfig

/**
 * Credential inputs needed to attach projects, besides the password.
 *
 * A project identifies accounts by user name if [ProjectConfig.usesName] is set and by email
 * address otherwise.
 */
data class CredentialInputs(
    val emailAddress: Boolean,
    val userName: Boolean,
    val minPwdLength: Int = DEFAULT_MIN_PWD_LENGTH
) {
    @StringRes
    fun findError(email: String, user: String, pwd: String): Int = when {
        emailAddress && email.isBlank() -> R.string.attachproject_error_no_email
        userName && user.isBlank() -> R.string.attachproject_error_no_name
        pwd.isEmpty() -> R.string.attachproject_error_no_pwd
        pwd.length < minPwdLength -> R.string.attachproject_error_short_pwd
        else -> 0
    }

    companion object {
        const val DEFAULT_MIN_PWD_LENGTH = 6

        // used as long as the project configurations are not known
        val ALL = CredentialInputs(emailAddress = true, userName = true)

        /**
         * Returns the inputs needed to attach the projects with [configs].
         * Account creation ([login] = false) always requires an email address.
         */
        fun forProjects(configs: Collection<ProjectConfig>, login: Boolean = true) = if (configs.isEmpty()) {
            ALL
        } else {
            CredentialInputs(
                emailAddress = !login || configs.any { !it.usesName },
                userName = configs.any { it.usesName },
                minPwdLength = configs.maxOf {
                    if (it.minPwdLength > 0) it.minPwdLength else DEFAULT_MIN_PWD_LENGTH
                }
            )
        }
    }
}
