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

import android.content.Intent
import android.graphics.Bitmap
import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.core.net.toUri
import androidx.core.text.parseAsHtml
import androidx.recyclerview.widget.RecyclerView
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.NoticesFragment
import edu.berkeley.boinc.R
import edu.berkeley.boinc.databinding.NoticesLayoutListItemBinding
import edu.berkeley.boinc.rpc.Notice
import edu.berkeley.boinc.utils.Logging
import edu.berkeley.boinc.utils.secondsToLocalDateTime
import java.time.format.DateTimeFormatter
import java.time.format.FormatStyle

class NoticesRecyclerViewAdapter(
        private val fragment: NoticesFragment,
        private val notices: List<Notice>
) : RecyclerView.Adapter<NoticesRecyclerViewAdapter.ViewHolder>() {
    private val dateTimeFormatter = DateTimeFormatter.ofLocalizedDateTime(FormatStyle.LONG,
            FormatStyle.SHORT)

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val binding = NoticesLayoutListItemBinding.inflate(LayoutInflater.from(parent.context))
        return ViewHolder(binding)
    }

    override fun getItemCount() = notices.size

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val listItem = notices[position]

        holder.root.setOnClickListener {
            val link = listItem.link

            Logging.logDebug(Logging.Category.USER_ACTION, "noticeClick: $link")

            if (link.isNotEmpty()) {
                fragment.requireActivity().startActivity(Intent(Intent.ACTION_VIEW, link.toUri()))
            }
        }

        val icon = getIcon(position)
        // if available set icon, if not boinc logo
        if (icon == null) {
            holder.projectIcon.setImageResource(R.drawable.ic_boinc)
        } else {
            holder.projectIcon.setImageBitmap(icon)
        }

        holder.projectName.text = listItem.projectName
        holder.title.text = listItem.title
        holder.content.text = listItem.description.parseAsHtml()

        holder.time.text = dateTimeFormatter.format(listItem.createTime.toLong().secondsToLocalDateTime())
    }

    private fun getIcon(position: Int): Bitmap? {
        return try {
            BOINCActivity.monitor!!.getProjectIconByName(notices[position].projectName)
        } catch (e: Exception) {
            Logging.logException(Logging.Category.MONITOR, "TasksListAdapter: Could not load data, clientStatus not initialized.", e)

            null
        }
    }

    inner class ViewHolder(binding: NoticesLayoutListItemBinding) : RecyclerView.ViewHolder(binding.root) {
        val root = binding.root
        val projectIcon = binding.projectIcon
        val projectName = binding.projectName
        val title = binding.noticeTitle
        val content = binding.noticeContent
        val time = binding.noticeTime
    }
}
