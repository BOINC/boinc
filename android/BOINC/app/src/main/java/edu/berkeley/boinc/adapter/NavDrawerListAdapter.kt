/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2021 University of California
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
import android.graphics.Bitmap
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.BaseAdapter
import android.widget.ImageView
import android.widget.TextView
import androidx.annotation.VisibleForTesting
import androidx.core.content.getSystemService
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.R
import edu.berkeley.boinc.client.IMonitor
import edu.berkeley.boinc.rpc.Project
import edu.berkeley.boinc.utils.Logging
import java.text.NumberFormat
import java.util.*

class NavDrawerListAdapter(val context: Context) : BaseAdapter() {
    private val navDrawerItems: MutableList<NavDrawerItem> = ArrayList()
    var selectedMenuId = 0
    override fun getCount(): Int {
        return navDrawerItems.size
    }

    override fun getItem(position: Int): NavDrawerItem {
        return navDrawerItems[position]
    }

    override fun getItemId(position: Int): Long {
        return navDrawerItems[position].id.toLong()
    }

    fun getItemForId(id: Int): NavDrawerItem? {
        return navDrawerItems.firstOrNull { it.id == id }
    }

    override fun getView(position: Int, convertView: View?, parent: ViewGroup): View {
        Logging.logDebug(Logging.Category.USER_ACTION,
                "NavDrawerListAdapter.getView() for ${navDrawerItems[position].title}:" +
                " counterVisibility: ${navDrawerItems[position].counterVisibility}" +
                " isSubItem: ${navDrawerItems[position].isSubItem}" +
                " isProjectItem: ${navDrawerItems[position].isProjectItem}"
        )
        val view : View
        if (convertView == null || convertView.tag == null || convertView.tag != navDrawerItems[position].title) {
            var layoutId = R.layout.navlist_listitem
            if (navDrawerItems[position].isSubItem) {
                layoutId = R.layout.navlist_listitem_subitem
            }
            val mInflater = context.getSystemService<LayoutInflater>()
            view = mInflater!!.inflate(layoutId, null)
        } else {
            view = convertView
        }
        val imgIcon = view.findViewById<ImageView>(R.id.icon)
        val txtTitle = view.findViewById<TextView>(R.id.title)
        val txtCount = view.findViewById<TextView>(R.id.counter)
        if (navDrawerItems[position].isProjectItem) {
            val icon = navDrawerItems[position].projectIcon
            if (icon == null) {
                navDrawerItems[position].updateProjectIcon()
            }
            if (icon != null) {
                imgIcon.setImageBitmap(icon)
            }
            val projectName = navDrawerItems[position].title
            if (projectName.isEmpty()) {
                navDrawerItems[position].updateProjectName()
            }
        } else {
            imgIcon.setImageResource(navDrawerItems[position].icon)
        }
        txtTitle.text = navDrawerItems[position].title

        // displaying count
        // check whether it set visible or not
        if (navDrawerItems[position].counterVisibility) {
            var counter = 0
            when (navDrawerItems[position].id) {
                R.string.tab_tasks -> try {
                    val monitor: IMonitor? = BOINCActivity.monitor
                    if (monitor != null) counter = monitor.tasksCount
                } catch (e: Exception) {
                    Logging.logException(Logging.Category.GUI_VIEW, "NavDrawerListAdapter.getView error: ", e)
                }
                R.string.tab_notices -> try {
                    val monitor: IMonitor? = BOINCActivity.monitor
                    if (monitor != null) counter = monitor.rssNotices.size
                } catch (e: Exception) {
                    Logging.logException(Logging.Category.GUI_VIEW, "NavDrawerListAdapter.getView error: ", e)
                }
            }
            txtCount.text = NumberFormat.getIntegerInstance().format(counter.toLong())
        } else {
            // hide the counter view
            txtCount.visibility = View.GONE
        }
        view.tag = navDrawerItems[position].title
        return view
    }

    fun getProjectIconForMasterUrl(masterUrl: String?): Bitmap? {
        try {
            val monitor: IMonitor? = BOINCActivity.monitor
            if (monitor != null && masterUrl != null) return monitor.getProjectIcon(masterUrl)
        } catch (e: Exception) {
            Logging.logException(Logging.Category.GUI_VIEW, "NavDrawerListAdapter.getProjectIconForMasterUrl error: ", e)
        }
        return null
    }

    fun getProjectNameForMasterUrl(masterUrl: String?): String {
        try {
            val monitor = BOINCActivity.monitor
            if (monitor != null && masterUrl != null) {
                val pi = monitor.getProjectInfoAsync(masterUrl, null).await()
                if (pi != null) {
                    return pi.name
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.GUI_VIEW, "NavDrawerListAdapter.getProjectNameForMasterUrl error: ", e)
        }
        return ""
    }

    /**
     * Compares list of projects to items represented in nav bar.
     *
     * @param projects Project list
     * @return Returns number of project items in nav bar after adding
     */
    fun compareAndAddProjects(projects: List<Project>?): Int {
        if (projects == null) {
            return 0
        }
        // delete all old projects from nav items
        navDrawerItems.removeIf { item: NavDrawerItem -> item.isProjectItem }
        var numberAdded = 0
        for ((masterURL, _, _, projectName) in projects) {
            val newProjectItem = NavDrawerItem(
                projectName,
                getProjectIconForMasterUrl(masterURL),
                masterURL
            )
            navDrawerItems.add(3, newProjectItem)
            numberAdded++
        }
        Logging.logDebug(Logging.Category.GUI_VIEW, "NavDrawerListAdapter.compareAndAddProjects() added: $numberAdded")
        notifyDataSetChanged()
        return numberAdded
    }

    fun updateUseAccountManagerItem() {
        val item = getItemForId(R.string.attachproject_acctmgr_header)
        var projectAddItemIndex = -1
        for (i in navDrawerItems.indices) {
            if (getItem(i).title == context.getString(R.string.projects_add)) {
                projectAddItemIndex = i
                break
            }
        }
        if (item != null && isAccountManagerPresent) {
            navDrawerItems.remove(item)
            notifyDataSetChanged()
        }
        if (item == null && !isAccountManagerPresent) {
            navDrawerItems.add(
                projectAddItemIndex + 1, NavDrawerItem(
                    R.string.attachproject_acctmgr_header,
                    R.drawable.ic_account_manager
                )
            )
            notifyDataSetChanged()
        }
    }

    // data retrieval failed, continue...
    @VisibleForTesting
    val isAccountManagerPresent: Boolean
        get() {
            try {
                val monitor: IMonitor? = BOINCActivity.monitor
                if (monitor != null) {
                    return monitor.clientAcctMgrInfo.isPresent
                }
            } catch (e: Exception) {
                // data retrieval failed, continue...
                Logging.logError(Logging.Category.MONITOR, "AcctMgrInfo data retrieval failed.")
            }
            return false
        }

    inner class NavDrawerItem {
        var id: Int
            private set
        var title: String
        var icon = 0
        var counterVisibility = false
            private set
        var isSubItem = false
            private set
        var isProjectItem = false
            private set

        @VisibleForTesting
        var projectIcon: Bitmap? = null
            private set
        var projectMasterUrl: String? = null
            private set

        /**
         * Creates default item
         */
        internal constructor(id: Int, icon: Int) {
            this.id = id
            title = context.getString(id)
            this.icon = icon
        }

        /**
         * Creates sub item under previous element
         */
        internal constructor(id: Int, icon: Int, isCounterVisible: Boolean, isSubItem: Boolean) {
            this.id = id
            title = context.getString(id)
            this.icon = icon
            this.isSubItem = isSubItem
            counterVisibility = isCounterVisible
        }

        /**
         * Creates item for project, which is sub item of Projects by default
         */
        internal constructor(name: String, icon: Bitmap?, masterUrl: String) {
            id = masterUrl.hashCode()
            title = name
            projectIcon = icon
            projectMasterUrl = masterUrl
            isProjectItem = true
            isSubItem = true
            Logging.logDebug(Logging.Category.GUI_VIEW, "NavDrawerItem: created hash code $id for project $name")
        }

        /**
         * Creates item with number counter on right
         */
        internal constructor(id: Int, icon: Int, isCounterVisible: Boolean) {
            this.id = id
            title = context.getString(id)
            this.icon = icon
            counterVisibility = isCounterVisible
        }

        fun updateProjectIcon() {
            projectIcon = getProjectIconForMasterUrl(projectMasterUrl)
        }

        fun updateProjectName() {
            title = getProjectNameForMasterUrl(projectMasterUrl)
        }
    }

    init {

        // populate items
        navDrawerItems.add(NavDrawerItem(R.string.tab_tasks, R.drawable.ic_baseline_list, true))
        navDrawerItems.add(
            NavDrawerItem(
                R.string.tab_notices,
                R.drawable.ic_baseline_email, true
            )
        )
        navDrawerItems.add(NavDrawerItem(R.string.tab_projects, R.drawable.ic_projects))
        navDrawerItems.add(
            NavDrawerItem(
                R.string.projects_add,
                R.drawable.ic_baseline_add_box, isCounterVisible = false, isSubItem = true
            )
        )
        navDrawerItems.add(
            NavDrawerItem(
                R.string.tab_preferences,
                R.drawable.ic_baseline_settings
            )
        )
        navDrawerItems.add(NavDrawerItem(R.string.menu_help, R.drawable.ic_baseline_help))
        navDrawerItems.add(
            NavDrawerItem(
                R.string.menu_report_issue,
                R.drawable.ic_baseline_bug_report
            )
        )
        navDrawerItems.add(NavDrawerItem(R.string.menu_about, R.drawable.ic_baseline_info))
        navDrawerItems.add(NavDrawerItem(R.string.menu_eventlog, R.drawable.ic_baseline_warning))
    }
}
