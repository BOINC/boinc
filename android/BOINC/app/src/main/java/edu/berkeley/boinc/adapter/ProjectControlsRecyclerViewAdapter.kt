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
import android.widget.TextView
import androidx.appcompat.content.res.AppCompatResources
import androidx.recyclerview.widget.RecyclerView
import edu.berkeley.boinc.ProjectsFragment
import edu.berkeley.boinc.ProjectsFragment.ProjectControl
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.RpcClient

class ProjectControlsRecyclerViewAdapter(
    var context: Context,
    // ID of control texts in strings.xml
    var entries: List<ProjectControl>
) : RecyclerView.Adapter<ProjectControlsRecyclerViewAdapter.ViewHolder>() {

    /**
     * This [ViewHolder] method describes an item view and metadata about its place within the RecyclerView
     *
     * @param itemView View
     */
    inner class ViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        //        declare UI components
        val tvText: TextView = itemView.findViewById(R.id.text)
    }

    /**
     * This override [onCreateViewHolder] method inflate recycler view
     *
     * @param parent view group
     * @param viewType view type
     * @return [ProjectControlsRecyclerViewAdapter.ViewHolder]
     */
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val context = parent.context
        val inflater = LayoutInflater.from(context)

        val controlItemView =
            inflater.inflate(R.layout.projects_controls_listitem_layout, parent, false)
        return ViewHolder(controlItemView)
    }

    /**
     * This overridden [onBindViewHolder] method add items to recycler view
     *
     * @param holder [ProjectControlsRecyclerViewAdapter.ViewHolder]
     * @param position position on each respective item
     */
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val data = entries[position]
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
                holder.tvText.background =
                    AppCompatResources.getDrawable(context, R.drawable.shape_light_red_background)
                text = context.resources.getString(R.string.projects_control_remove)
            }
            RpcClient.MGR_SYNC -> text =
                context.resources.getString(R.string.projects_control_sync_acctmgr)
            RpcClient.MGR_DETACH -> {
                holder.tvText.background =
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

        holder.tvText.setOnClickListener(entries[position].projectCommandClickListener)
        holder.tvText.text = text
    }

    /**
     * This overridden [getItemCount] method return item count
     *
     * @return [Int] no of items in given list
     */
    override fun getItemCount(): Int {
        return entries.size
    }
}
