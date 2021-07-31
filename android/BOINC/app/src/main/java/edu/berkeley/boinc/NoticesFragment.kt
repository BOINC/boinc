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
package edu.berkeley.boinc

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.LinearLayoutManager
import edu.berkeley.boinc.adapter.NoticesRecyclerViewAdapter
import edu.berkeley.boinc.databinding.NoticesLayoutBinding
import edu.berkeley.boinc.rpc.Notice
import edu.berkeley.boinc.utils.Logging

class NoticesFragment : Fragment() {
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")
    private val mClientStatusChangeRec = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            Logging.logVerbose(Logging.Category.GUI_VIEW, "NoticesFragment ClientStatusChange - onReceive()")

            // data retrieval
            val notices = updateNotices()
            data.clear()
            data.addAll(notices)
            noticesRecyclerViewAdapter.notifyDataSetChanged()
        }
    }

    private lateinit var noticesRecyclerViewAdapter: NoticesRecyclerViewAdapter
    private var data: MutableList<Notice> = ArrayList()

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        Logging.logVerbose(Logging.Category.GUI_VIEW, "NoticesFragment onCreateView")

        val binding = NoticesLayoutBinding.inflate(inflater, container, false)

        noticesRecyclerViewAdapter = NoticesRecyclerViewAdapter(this, data)
        binding.noticesList.adapter = noticesRecyclerViewAdapter
        binding.noticesList.layoutManager = LinearLayoutManager(context)
        return binding.root
    }

    override fun onResume() {
        Logging.logVerbose(Logging.Category.GUI_VIEW, "NoticesFragment onResume()")

        activity?.registerReceiver(mClientStatusChangeRec, ifcsc)

        // clear notice notification
        try {
            BOINCActivity.monitor!!.cancelNoticeNotification()
        } catch (e: Exception) {
            Logging.logException(Logging.Category.GUI_VIEW, "NoticesFragment.onResume error: ", e)
        }
        super.onResume()
    }

    override fun onPause() {
        //unregister receiver, so there are not multiple intents flying in
        Logging.logVerbose(Logging.Category.GUI_VIEW, "NoticesFragment remove receiver")

        activity?.unregisterReceiver(mClientStatusChangeRec)
        super.onPause()
    }

    private fun updateNotices(): List<Notice> {
        return try {
            BOINCActivity.monitor!!.rssNotices.sortedWith(compareBy<Notice> { it.createTime }
                    .reversed())
        } catch (e: Exception) {
            Logging.logException(Logging.Category.GUI_VIEW, "NoticesFragment.updateNotices error: ", e)

            emptyList()
        }
    }
}
