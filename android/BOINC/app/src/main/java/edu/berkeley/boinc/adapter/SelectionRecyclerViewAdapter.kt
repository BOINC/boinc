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

import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import edu.berkeley.boinc.R
import edu.berkeley.boinc.attach.AcctMgrFragment
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

        if (listItem.isAccountManager) {
            // element is account manager
            holder.name.text = activity.getString(R.string.attachproject_acctmgr_header)
            holder.description.text = activity.getString(R.string.attachproject_acctmgr_list_desc)
            holder.checkBox.visibility = View.GONE
            holder.summary.visibility = View.GONE
            holder.accountManagerButtonImage.visibility = View.VISIBLE
            val listener = View.OnClickListener {
                if (Logging.DEBUG) {
                    Log.d(Logging.TAG, "SelectionListAdapter: account manager clicked.")
                }
                // configure so dialog returns to main activity when finished
                val dialog = AcctMgrFragment().apply { setReturnToMainActivity() }
                dialog.show(activity.supportFragmentManager,
                        activity.getString(R.string.attachproject_acctmgr_header))
            }
            holder.root.setOnClickListener(listener)
            holder.name.setOnClickListener(listener)
            holder.description.setOnClickListener(listener)
            holder.accountManagerButtonImage.setOnClickListener(listener)
        } else {
            // element is project option
            holder.name.text = listItem.info?.name
            holder.description.text = listItem.info?.generalArea
            holder.summary.text = listItem.info?.summary
            holder.checkBox.isChecked = listItem.isChecked
            holder.checkBox.setOnClickListener { listItem.isChecked = !listItem.isChecked }
            holder.textWrapper.setOnClickListener {
                if (Logging.DEBUG) {
                    Log.d(Logging.TAG, "SelectionListAdapter: onProjectClick open info for: " +
                            listItem.info?.name)
                }
                val dialog = newInstance(listItem.info)
                dialog.show(activity.supportFragmentManager, "ProjectInfoFragment")
            }
        }
    }

    inner class ViewHolder(binding: AttachProjectListLayoutListItemBinding) :
            RecyclerView.ViewHolder(binding.root) {
        val root = binding.root
        val name = binding.name
        val description = binding.description
        val summary = binding.summary
        val checkBox = binding.checkBox
        val accountManagerButtonImage = binding.amButtonImage
        val textWrapper = binding.textWrapper
    }
}
