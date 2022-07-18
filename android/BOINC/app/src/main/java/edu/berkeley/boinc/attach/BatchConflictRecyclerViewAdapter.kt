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
package edu.berkeley.boinc.attach

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.FragmentActivity
import androidx.recyclerview.widget.RecyclerView
import edu.berkeley.boinc.R
import edu.berkeley.boinc.databinding.AttachProjectBatchConflictsListItemBinding
import edu.berkeley.boinc.utils.Logging

class BatchConflictRecyclerViewAdapter(
        private val activity: FragmentActivity,
        private val entries: List<ProjectAttachService.ProjectAttachWrapper>
): RecyclerView.Adapter<BatchConflictRecyclerViewAdapter.ViewHolder>() {
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val binding = AttachProjectBatchConflictsListItemBinding.inflate(LayoutInflater.from(parent.context))
        return ViewHolder(binding)
    }

    override fun getItemCount() = entries.size

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val listItem = entries[position]

        Logging.logDebug(Logging.Category.GUI_VIEW, "BatchConflictListAdapter.getView for: ${listItem.name} at" +
                " position: $position with result: ${listItem.result}")

        holder.name.text = listItem.name
        if (listItem.result == ProjectAttachService.RESULT_SUCCESS) {
            // success
            holder.status.visibility = View.GONE
            holder.resolveButtonImage.visibility = View.GONE
            holder.statusProgressBar.visibility = View.GONE
            holder.statusImage.visibility = View.VISIBLE
            holder.statusImage.setImageResource(R.drawable.ic_baseline_check)
        } else if (listItem.result == ProjectAttachService.RESULT_ONGOING ||
                listItem.result == ProjectAttachService.RESULT_UNINITIALIZED) {
            // ongoing
            holder.status.visibility = View.GONE
            holder.resolveButtonImage.visibility = View.GONE
            holder.statusImage.visibility = View.GONE
            holder.statusProgressBar.visibility = View.VISIBLE
        } else if (listItem.result == ProjectAttachService.RESULT_READY) {
            // ready
            holder.status.visibility = View.VISIBLE
            holder.status.text = listItem.resultDescription
            holder.resolveButtonImage.visibility = View.VISIBLE
            holder.resolveItemWrapper.setOnClickListener {
                Logging.logVerbose(Logging.Category.USER_ACTION, "BatchConflictListAdapter: start resolution dialog for: ${listItem.name}")

                val dialog = IndividualCredentialInputFragment.newInstance(listItem)
                dialog.show(activity.supportFragmentManager, listItem.name)
            }
        } else if (listItem.result == ProjectAttachService.RESULT_CONFIG_DOWNLOAD_FAILED) {
            // download failed, can not continue from here.
            // if user wants to retry, need to go back to selection activity
            holder.status.visibility = View.VISIBLE
            holder.status.text = listItem.resultDescription
            holder.resolveButtonImage.visibility = View.GONE
            holder.statusProgressBar.visibility = View.GONE
            holder.statusImage.visibility = View.VISIBLE
            holder.statusImage.setImageResource(R.drawable.ic_baseline_clear)
        } else {
            // failed
            holder.status.visibility = View.VISIBLE
            holder.status.text = listItem.resultDescription
            holder.resolveButtonImage.visibility = View.VISIBLE
            holder.resolveItemWrapper.setOnClickListener {
                Logging.logVerbose(Logging.Category.USER_ACTION, "BatchConflictListAdapter: start resolution dialog for: ${listItem.name}")

                val dialog = IndividualCredentialInputFragment.newInstance(listItem)
                dialog.show(activity.supportFragmentManager, listItem.name)
            }
            holder.statusProgressBar.visibility = View.GONE
            holder.statusImage.visibility = View.VISIBLE
            holder.statusImage.setImageResource(R.drawable.ic_baseline_clear)
        }
    }

    class ViewHolder(binding: AttachProjectBatchConflictsListItemBinding):
            RecyclerView.ViewHolder(binding.root) {
        val name = binding.name
        val status = binding.status
        val resolveButtonImage = binding.resolveButtonImage
        val statusImage = binding.statusImage
        val statusProgressBar = binding.statusProgressBar
        val resolveItemWrapper = binding.resolveItemWrapper
    }
}
