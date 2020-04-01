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

import android.graphics.Bitmap
import android.util.Log
import edu.berkeley.boinc.utils.Logging

class NavDrawerItem internal constructor(
        private val navDrawerListAdapter: NavDrawerListAdapter,
        var id: Int,
        val icon: Int
) {
    var title: String
    var counterVisibility = false
        private set
    var isSubItem = false
        private set
    var isProjectItem = false
        private set
    var projectIcon: Bitmap? = null
        private set
    var projectMasterUrl: String? = null
        private set

    /**
     * Creates default item
     */
    init {
        title = navDrawerListAdapter.context.getString(id)
    }

    /**
     * Creates item with number counter on right
     */
    internal constructor(
            navDrawerListAdapter: NavDrawerListAdapter,
            id: Int,
            icon: Int,
            isCounterVisible: Boolean
    ) : this(navDrawerListAdapter, id, icon) {
        counterVisibility = isCounterVisible
    }

    /**
     * Creates sub item under previous element
     */
    internal constructor(
            navDrawerListAdapter: NavDrawerListAdapter,
            id: Int,
            icon: Int,
            isCounterVisible: Boolean,
            isSubItem: Boolean
    ) : this(navDrawerListAdapter, id, icon, isCounterVisible) {
        this.isSubItem = isSubItem
    }

    /**
     * Creates item for project, which is sub item of Projects by default
     */
    internal constructor(
            navDrawerListAdapter: NavDrawerListAdapter,
            name: String,
            icon: Bitmap?,
            masterUrl: String
    ) : this(navDrawerListAdapter, 0, 0, true) {
        id = masterUrl.hashCode()
        title = name
        projectIcon = icon
        projectMasterUrl = masterUrl
        isProjectItem = true
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "NavDrawerItem: created hash code $id for project $name")
        }
    }

    fun updateProjectIcon() {
        projectIcon = navDrawerListAdapter.getProjectIconForMasterUrl(projectMasterUrl)
    }
}
