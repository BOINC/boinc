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

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import edu.berkeley.boinc.attach.ProjectInfoFragment.Companion.newInstance
import edu.berkeley.boinc.attach.SelectionListActivity
import edu.berkeley.boinc.databinding.AttachProjectListLayoutListItemBinding
import edu.berkeley.boinc.utils.Logging

class SelectionRecyclerViewAdapter(
        private val activity: SelectionListActivity,
        private val entries: List<ProjectListEntry>
) : RecyclerView.Adapter<SelectionRecyclerViewAdapter.ViewHolder>() {
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val binding = AttachProjectListLayoutListItemBinding.inflate(LayoutInflater.from(parent.context))
        return ViewHolder(binding)
    }

    override fun getItemCount() = entries.size

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val listItem = entries[position]

        // element is project option
        holder.name.text = listItem.info?.name
        holder.description.text = listItem.info?.generalArea
        holder.summary.text = listItem.info?.summary
        holder.checkBox.isChecked = listItem.isChecked
        holder.checkBox.setOnClickListener { listItem.isChecked = !listItem.isChecked }
        holder.textWrapper.setOnClickListener {
            Logging.logDebug(Logging.Category.USER_ACTION, "SelectionListAdapter: onProjectClick open info for: " +
                    listItem.info?.name)

            val dialog = newInstance(listItem.info)
            dialog.show(activity.supportFragmentManager, "ProjectInfoFragment")
        }
    }

    inner class ViewHolder(binding: AttachProjectListLayoutListItemBinding) :
            RecyclerView.ViewHolder(binding.root) {
        val root = binding.root
        val name = binding.name
        val description = binding.description
        val summary = binding.summary
        val checkBox = binding.checkBox
        val textWrapper = binding.textWrapper
    }
}
