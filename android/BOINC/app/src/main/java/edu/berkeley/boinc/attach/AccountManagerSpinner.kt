/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2018 University of California
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

/**
 * Used in Spinner list to show the account manager's name but use its url as value.
 */
class AccountManagerSpinner(var name: String, var url: String) {
    /**
     * Returns what to display in the Spinner list.
     * @return The name of account manager.
     */
    override fun toString() = name
}
