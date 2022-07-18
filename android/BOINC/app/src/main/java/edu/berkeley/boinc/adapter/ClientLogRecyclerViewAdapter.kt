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

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import edu.berkeley.boinc.databinding.EventLogClientListItemLayoutBinding
import edu.berkeley.boinc.rpc.Message
import edu.berkeley.boinc.utils.secondsToLocalDateTime
import java.time.format.DateTimeFormatter
import java.time.format.FormatStyle

class ClientLogRecyclerViewAdapter(private val messages: List<Message>) :
        RecyclerView.Adapter<ClientLogRecyclerViewAdapter.ViewHolder>() {
    private val dateTimeFormatter = DateTimeFormatter.ofLocalizedDateTime(FormatStyle.MEDIUM)

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val binding = EventLogClientListItemLayoutBinding.inflate(LayoutInflater.from(parent.context))
        return ViewHolder(binding)
    }

    override fun getItemCount() = messages.size

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.date.text = getDateTimeString(position)
        holder.message.text = getMessage(position)

        val project = getProject(position)
        if (project.isEmpty()) {
            holder.project.visibility = View.GONE
        } else {
            holder.project.visibility = View.VISIBLE
            holder.project.text = project
        }
    }

    fun getDateTimeString(position: Int): String = dateTimeFormatter.format(messages[position].timestamp.secondsToLocalDateTime())

    fun getMessage(position: Int) = messages[position].body

    fun getProject(position: Int) = messages[position].project

    class ViewHolder(binding: EventLogClientListItemLayoutBinding) : RecyclerView.ViewHolder(binding.root) {
        val date = binding.msgsDate
        val project = binding.msgsProject
        val message = binding.msgsMessage
    }
}
