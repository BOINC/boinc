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
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import edu.berkeley.boinc.attach.AttachActivity
import edu.berkeley.boinc.databinding.AttachProjectListLayoutListItemBinding
import edu.berkeley.boinc.utils.Logging

class AttachActivityItemAdapter(
        private val attachActivityItems: List<AttachActivityItem>,
        private val activity: AttachActivity
) : RecyclerView.Adapter<AttachActivityItemAdapter.ViewHolder>() {
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val binding = AttachProjectListLayoutListItemBinding.inflate(LayoutInflater.from(parent.context))
        return ViewHolder(binding)
    }

    override fun getItemCount(): Int = attachActivityItems.count()

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val attachActivityItem = attachActivityItems[position]

        holder.name.text = attachActivityItem.text
        holder.description.text = attachActivityItem.description
        holder.checkBox.visibility = View.GONE
        holder.summary.visibility = View.GONE
        holder.buttonImage.visibility = View.VISIBLE
        val listener = View.OnClickListener {
            when (attachActivityItem.type) {
                AttachActivityItemType.ACCOUNT_MANAGER -> {
                    Logging.logVerbose(Logging.Category.USER_ACTION, "AttachActivityItemAdapter: account manager clicked.")

                    activity.startAccountManagerActivity()
                }
                AttachActivityItemType.ALL_PROJECTS -> {
                    Logging.logVerbose(Logging.Category.USER_ACTION, "AttachActivityItemAdapter: projects clicked.")

                    activity.startSelectionProjectActivity()
                }
            }
        }
        holder.root.setOnClickListener(listener)
        holder.name.setOnClickListener(listener)
        holder.description.setOnClickListener(listener)
        holder.buttonImage.setOnClickListener(listener)
     }


    inner class ViewHolder(binding: AttachProjectListLayoutListItemBinding) :
            RecyclerView.ViewHolder(binding.root) {
        val root = binding.root
        val name = binding.name
        val description = binding.description
        val summary = binding.summary
        val checkBox = binding.checkBox
        val buttonImage = binding.amButtonImage
    }
}
