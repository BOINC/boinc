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
package edu.berkeley.boinc

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import edu.berkeley.boinc.adapter.NoticesListAdapter
import edu.berkeley.boinc.databinding.NoticesLayoutBinding
import edu.berkeley.boinc.rpc.Notice
import edu.berkeley.boinc.utils.Logging

class NoticesFragment : Fragment() {
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")
    private val mClientStatusChangeRec = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            if (Logging.VERBOSE) {
                Log.d(Logging.TAG, "NoticesFragment ClientStatusChange - onReceive()")
            }

            // data retrieval
            updateNotices()
            noticesListAdapter.clear()
            noticesListAdapter.addAll(data)
            noticesListAdapter.notifyDataSetChanged()
        }
    }

    private lateinit var noticesListAdapter: NoticesListAdapter
    private var data: MutableList<Notice> = ArrayList()

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (Logging.VERBOSE) {
            Log.d(Logging.TAG, "NoticesFragment onCreateView")
        }
        val binding = NoticesLayoutBinding.inflate(inflater, container, false)
        updateNotices()

        noticesListAdapter = NoticesListAdapter(activity, R.id.noticesList, data)
        binding.noticesList.adapter = noticesListAdapter
        return binding.root
    }

    override fun onResume() {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "NoticesFragment onResume()")
        }
        activity?.registerReceiver(mClientStatusChangeRec, ifcsc)

        // clear notice notification
        try {
            BOINCActivity.monitor!!.cancelNoticeNotification()
        } catch (e: Exception) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "NoticesFragment.onResume error: ", e)
            }
        }
        super.onResume()
    }

    override fun onPause() {
        //unregister receiver, so there are not multiple intents flying in
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "NoticesFragment remove receiver")
        }
        activity?.unregisterReceiver(mClientStatusChangeRec)
        super.onPause()
    }

    private fun updateNotices() {
        try {
            data = BOINCActivity.monitor!!.rssNotices
            // sorting policy: latest arrival first.
            data.sortWith(compareBy<Notice> { it.createTime }.reversed())
        } catch (e: Exception) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "NoticesFragment.updateNotices error: ", e)
            }
        }
    }
}
