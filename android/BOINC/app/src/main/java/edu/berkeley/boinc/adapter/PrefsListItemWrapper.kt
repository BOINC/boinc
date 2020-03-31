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
package edu.berkeley.boinc.adapter

import android.content.Context
import edu.berkeley.boinc.R

open class PrefsListItemWrapper(val context: Context, val id: Int, val isCategory: Boolean) {
    enum class DialogButtonType {
        SLIDER, NUMBER, TEXT
    }

    var dialogButtonType: DialogButtonType? = null
    var header: String? = null
    var description = ""

    // Constructor for elements
    constructor(context: Context, id: Int) : this(context, id, false) {
        header = context.getString(id)
        if (id == R.string.prefs_power_source_header) {
            description = context.getString(R.string.prefs_power_source_description)
        }
        // further description mapping see PrefsListItemWrapperNumber class
    }
}
