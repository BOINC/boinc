/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2025 University of California
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
package edu.berkeley.boinc.ui.eventlog

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.LinearLayoutManager
import edu.berkeley.boinc.R
import edu.berkeley.boinc.adapter.GuiLogRecyclerViewAdapter
import edu.berkeley.boinc.databinding.EventLogGuiLayoutBinding
import edu.berkeley.boinc.utils.Logging
import edu.berkeley.boinc.utils.Logging.Category.DEVICE
import edu.berkeley.boinc.utils.Logging.getLogLevel
import edu.berkeley.boinc.utils.Logging.logException
import edu.berkeley.boinc.utils.Logging.logVerbose
import edu.berkeley.boinc.utils.readLineLimit
import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader

class EventLogGuiFragment : Fragment() {
    private var a: EventLogActivity? = null
    private var binding: EventLogGuiLayoutBinding? = null
    private var adapter: GuiLogRecyclerViewAdapter? = null

    @Suppress("RedundantNullableReturnType")
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        a = (activity as EventLogActivity?)

        binding = EventLogGuiLayoutBinding.inflate(inflater, container, false)

        adapter = GuiLogRecyclerViewAdapter(a!!.guiLogData)
        binding!!.guiLogList.layoutManager = LinearLayoutManager(context)
        binding!!.guiLogList.adapter = adapter
        binding!!.root.setOnRefreshListener { this.readLogcat() }

        // read messages
        readLogcat()

        return binding!!.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding = null
    }

    fun update() {
        readLogcat()
    }

    private fun readLogcat() {
        val number = resources.getInteger(R.integer.eventlog_gui_messages)
        a!!.guiLogData.clear()
        try {
            var logLevelFilter = Logging.TAG
            when (getLogLevel()) {
                0 -> return
                1 -> logLevelFilter += ":E"
                2 -> logLevelFilter += ":W"
                3 -> logLevelFilter += ":I"
                4 -> logLevelFilter += ":D"
                5 -> logLevelFilter += ":V"
            }
            val process =
                Runtime.getRuntime().exec("logcat -d -t $number -v time $logLevelFilter *:S")
            // filtering logcat output by application package is not possible on command line
            // devices with SDK > 13 will automatically "session filter"
            val bufferedReader = BufferedReader(InputStreamReader(process.inputStream))

            var line: String?
            var x = 0
            while ((bufferedReader.readLineLimit(4096).also { line = it }) != null) {
                if (x > 1) {
                    a!!.guiLogData.add(
                        0,
                        line!!
                    ) // cut off first two lines, prepend to array (most current on top)
                }
                x++
            }

            logVerbose(DEVICE, "readLogcat read " + a!!.guiLogData.size + " lines.")

            adapter!!.notifyDataSetChanged()
            binding!!.root.isRefreshing = false
        } catch (e: IOException) {
            logException(DEVICE, "readLogcat failed", e)
        }
    }
}
