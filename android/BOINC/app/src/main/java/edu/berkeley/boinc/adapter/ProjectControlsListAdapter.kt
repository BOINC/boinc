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
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.TextView
import androidx.appcompat.content.res.AppCompatResources
import edu.berkeley.boinc.ProjectsFragment
import edu.berkeley.boinc.ProjectsFragment.ProjectControl
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.RpcClient

class ProjectControlsListAdapter(
    context: Context,
    // ID of control texts in strings.xml
    private val entries: List<ProjectControl>
) : ArrayAdapter<ProjectControl>(
    context, R.layout.projects_controls_listitem_layout, entries
) {
    override fun getCount(): Int {
        return entries.size
    }

    override fun getItem(position: Int): ProjectControl {
        return entries[position]
    }

    override fun getItemId(position: Int): Long {
        return entries[position].operation.toLong()
    }

    override fun getView(position: Int, convertView: View?, parent: ViewGroup): View {
        val data = entries[position]
        val vi = LayoutInflater.from(parent.context)
            .inflate(R.layout.projects_controls_listitem_layout, null)
        val tvText = vi.findViewById<TextView>(R.id.text)
        var text = ""
        when (data.operation) {
            RpcClient.PROJECT_UPDATE -> text =
                context.resources.getString(R.string.projects_control_update)
            RpcClient.PROJECT_SUSPEND -> text =
                context.resources.getString(R.string.projects_control_suspend)
            RpcClient.PROJECT_RESUME -> text =
                context.resources.getString(R.string.projects_control_resume)
            RpcClient.PROJECT_ANW -> text =
                context.resources.getString(R.string.projects_control_allownewtasks)
            RpcClient.PROJECT_NNW -> text =
                context.resources.getString(R.string.projects_control_nonewtasks)
            RpcClient.PROJECT_RESET -> text =
                context.resources.getString(R.string.projects_control_reset)
            RpcClient.PROJECT_DETACH -> {
                tvText.background =
                    AppCompatResources.getDrawable(context, R.drawable.shape_light_red_background)
                text = context.resources.getString(R.string.projects_control_remove)
            }
            RpcClient.MGR_SYNC -> text =
                context.resources.getString(R.string.projects_control_sync_acctmgr)
            RpcClient.MGR_DETACH -> {
                tvText.background =
                    AppCompatResources.getDrawable(context, R.drawable.shape_light_red_background)
                text = context.resources.getString(R.string.projects_control_remove_acctmgr)
            }
            RpcClient.TRANSFER_RETRY -> text =
                context.resources.getString(R.string.trans_control_retry)
            ProjectsFragment.VISIT_WEBSITE -> text =
                context.resources.getString(R.string.projects_control_visit_website)
            else -> {
            }
        }

        //set onclicklistener for expansion
        vi.setOnClickListener(entries[position].projectCommandClickListener)

        // set data of standard elements
        tvText.text = text
        return vi
    }
}
