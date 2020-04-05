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

import edu.berkeley.boinc.PrefsFragment

class SelectionDialogOption private constructor(
        var name: String,
        var id: Int,
        var isSelected: Boolean,
        val isHighlighted: Boolean
) {
    constructor(name: String) : this(name, 0, false, false)
    constructor(prefsFragment: PrefsFragment, id: Int, selected: Boolean) :
            this(prefsFragment.resources.getString(id), id, selected, false)
    constructor(prefsFragment: PrefsFragment, id: Int, selected: Boolean, highlighted: Boolean) :
            this(prefsFragment.resources.getString(id), id, selected, highlighted)
}
