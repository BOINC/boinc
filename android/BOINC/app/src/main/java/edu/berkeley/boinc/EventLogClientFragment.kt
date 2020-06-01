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

import android.os.Bundle
import android.os.RemoteException
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AbsListView
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import edu.berkeley.boinc.adapter.ClientLogListAdapter
import edu.berkeley.boinc.rpc.Message
import edu.berkeley.boinc.utils.Logging
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class EventLogClientFragment : Fragment() {
    private lateinit var activity: EventLogActivity

    private var mostRecentSeqNo = 0
    private var pastSeqNo = -1 // oldest (lowest) seqNo currently loaded to GUI

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        activity = getActivity() as EventLogActivity
        val layout = inflater.inflate(R.layout.eventlog_client_layout, container, false)
        activity.clientLogList = layout.findViewById(R.id.clientLogList)
        activity.clientLogListAdapter = ClientLogListAdapter(getActivity(), activity.clientLogList,
                R.id.clientLogList, activity.clientLogData)
        activity.clientLogList.setOnScrollListener(EndlessScrollListener(5))
        return layout
    }

    fun init() {
        lifecycleScope.launch {
            retrievePastClientMessages() // read messages
        }
    }

    fun update() {
        lifecycleScope.launch {
            retrieveRecentClientMessages() // refresh messages
        }
    }

    // onScrollListener for list view, implementing "endless scrolling"
    inner class EndlessScrollListener(private val visibleThreshold: Int) : AbsListView.OnScrollListener {
        private var previousTotal = 0
        private var loading = true

        override fun onScroll(view: AbsListView, firstVisibleItem: Int, visibleItemCount: Int, totalItemCount: Int) {
            if (loading && totalItemCount > previousTotal) {
                loading = false
                previousTotal = totalItemCount
            }
            if (!loading && totalItemCount - visibleItemCount <= firstVisibleItem + visibleThreshold) {
                lifecycleScope.launch {
                    retrievePastClientMessages()
                }
                loading = true
            }
        }

        override fun onScrollStateChanged(view: AbsListView, scrollState: Int) {}
    }

    private suspend fun retrieveRecentClientMessages() {
        if (activity.clientLogData.isNotEmpty()) {
            mostRecentSeqNo = activity.clientLogData[0].seqno
        }

        coroutineScope {
            val messages = withContext(Dispatchers.Default) {
                return@withContext try {
                    (getActivity() as EventLogActivity).monitorService.getMessages(mostRecentSeqNo)
                } catch (e: RemoteException) {
                    e.printStackTrace()
                    emptyList<Message>()
                }
            }

            try {
                for ((y, x) in messages.indices.reversed().withIndex()) {
                    activity.clientLogData.add(y, messages[x])
                }
            } catch (e: Exception) {
                if (Logging.ERROR) {
                    Log.e(Logging.TAG, "EventLogClientFragment.loadRecentMsgs error: ", e)
                }
            } //IndexOutOfBoundException
            activity.clientLogListAdapter.notifyDataSetChanged()
        }
    }

    private suspend fun retrievePastClientMessages() {
        if (activity.clientLogData.isNotEmpty()) {
            pastSeqNo = activity.clientLogData.last().seqno
            if (pastSeqNo == 0) {
                if (Logging.DEBUG) {
                    Log.d("RetrievePastMsgs", "cancel, oldest messages already loaded")
                }
                return // cancel if all past messages are present
            }
        }

        // message retrieval
        // amount messages loaded when end of list is reached
        val pastMsgsLoadingRange = 50
        if (Logging.DEBUG) {
            Log.d("RetrievePastMsgs", "calling monitor with: " + pastSeqNo + " / " +
                    pastMsgsLoadingRange)
        }

        coroutineScope {
            val messages = withContext(Dispatchers.Default) {
                return@withContext try {
                    (getActivity() as EventLogActivity).monitorService.getEventLogMessages(pastSeqNo,
                            pastMsgsLoadingRange)
                } catch (e: RemoteException) {
                    e.printStackTrace()
                    emptyList<Message>()
                }
            }

            // back in UI thread
            // Append old messages to the event log
            try {
                activity.clientLogData.addAll(messages.reversed())
            } catch (e: Exception) {
                if (Logging.ERROR) {
                    Log.e(Logging.TAG, "EventLogClientFragment.loadPastMsgs error: ", e)
                }
            } //IndexOutOfBoundException
            activity.clientLogListAdapter.notifyDataSetChanged()
        }
    }
}
