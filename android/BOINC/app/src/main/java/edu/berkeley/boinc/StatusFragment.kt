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
import android.os.RemoteException
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.TextView
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import edu.berkeley.boinc.attach.SelectionListActivity
import edu.berkeley.boinc.client.ClientStatus
import edu.berkeley.boinc.utils.Logging
import edu.berkeley.boinc.utils.RUN_MODE_AUTO
import edu.berkeley.boinc.utils.SUSPEND_REASON_BATTERIES
import edu.berkeley.boinc.utils.SUSPEND_REASON_BATTERY_CHARGING
import edu.berkeley.boinc.utils.SUSPEND_REASON_BATTERY_OVERHEATED
import edu.berkeley.boinc.utils.SUSPEND_REASON_BENCHMARKS
import edu.berkeley.boinc.utils.SUSPEND_REASON_USER_ACTIVE
import edu.berkeley.boinc.utils.SUSPEND_REASON_USER_REQ
import edu.berkeley.boinc.utils.writeClientModeAsync
import kotlinx.coroutines.launch

class StatusFragment : Fragment() {
    // keep computingStatus and suspend reason to only adapt layout when changes occur
    private var computingStatus = -1
    private var computingSuspendReason = -1
    private var networkSuspendReason = -1
    private var setupStatus = -1
    private val mClientStatusChangeRec: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            Logging.logVerbose(Logging.Category.GUI_VIEW, "StatusFragment ClientStatusChange - onReceive()")

            loadLayout()
        }
    }
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")
    override fun onResume() {
        //register noisy clientStatusChangeReceiver here, so only active when Activity is visible
        Logging.logVerbose(Logging.Category.GUI_VIEW, "StatusFragment register receiver")

        requireActivity().registerReceiver(mClientStatusChangeRec, ifcsc)
        super.onResume()
    }

    override fun onPause() {
        //unregister receiver, so there are not multiple intents flying in
        Logging.logVerbose(Logging.Category.GUI_VIEW, "StatusFragment remove receiver")

        requireActivity().unregisterReceiver(mClientStatusChangeRec)
        super.onPause()
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        Logging.logVerbose(Logging.Category.GUI_VIEW, "StatusFragment onCreateView")

        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.status_layout, container, false)
    }

    private fun loadLayout() {
        //load layout, if if ClientStatus can be accessed.
        //if this is not the case, the broadcast receiver will call "loadLayout" again
        try {
            val currentSetupStatus = BOINCActivity.monitor!!.setupStatus
            val currentComputingStatus = BOINCActivity.monitor!!.computingStatus
            val currentComputingSuspendReason = BOINCActivity.monitor!!.computingSuspendReason
            val currentNetworkSuspendReason = BOINCActivity.monitor!!.networkSuspendReason

            // layout only if client RPC connection is established
            // otherwise BOINCActivity does not start Tabs
            if (currentSetupStatus == ClientStatus.SETUP_STATUS_AVAILABLE) {
                // return in cases nothing has changed
                if (computingStatus != currentComputingStatus || currentComputingSuspendReason != computingSuspendReason || currentNetworkSuspendReason != networkSuspendReason) {
                    // set layout and retrieve elements
                    val statusWrapper = requireView().findViewById<LinearLayout>(R.id.status_wrapper)
                    val centerWrapper = requireView().findViewById<LinearLayout>(R.id.center_wrapper)
                    val restartingWrapper = requireView().findViewById<LinearLayout>(R.id.restarting_wrapper)
                    val statusHeader = requireView().findViewById<TextView>(R.id.status_header)
                    val statusImage = requireView().findViewById<ImageView>(R.id.status_image)
                    val statusDescriptor = requireView().findViewById<TextView>(R.id.status_long)
                    restartingWrapper.visibility = View.GONE
                    when (currentComputingStatus) {
                        ClientStatus.COMPUTING_STATUS_NEVER -> {
                            statusWrapper.visibility = View.VISIBLE
                            statusHeader.text = BOINCActivity.monitor!!.currentStatusTitle
                            statusHeader.visibility = View.VISIBLE
                            statusImage.setImageResource(R.drawable.ic_baseline_play_arrow)
                            statusImage.contentDescription = BOINCActivity.monitor!!.currentStatusTitle
                            statusDescriptor.text = BOINCActivity.monitor!!.currentStatusDescription
                            centerWrapper.visibility = View.VISIBLE
                            centerWrapper.setOnClickListener(runModeOnClickListener)
                        }
                        ClientStatus.COMPUTING_STATUS_SUSPENDED -> {
                            statusWrapper.visibility = View.VISIBLE
                            statusHeader.text = BOINCActivity.monitor!!.currentStatusTitle
                            statusHeader.visibility = View.VISIBLE
                            statusImage.setImageResource(R.drawable.ic_baseline_pause)
                            statusImage.contentDescription = BOINCActivity.monitor!!.currentStatusTitle
                            statusImage.isClickable = false
                            centerWrapper.visibility = View.VISIBLE
                            when (currentComputingSuspendReason) {
                                SUSPEND_REASON_BATTERIES -> {
                                    statusDescriptor.text = BOINCActivity.monitor!!.currentStatusDescription
                                    statusImage.setImageResource(R.drawable.ic_baseline_power_off)
                                    statusHeader.visibility = View.GONE
                                }
                                SUSPEND_REASON_USER_ACTIVE -> {
                                    var suspendDueToScreenOn = false
                                    try {
                                        suspendDueToScreenOn = BOINCActivity.monitor!!.suspendWhenScreenOn
                                    } catch (e: RemoteException) {
                                        Logging.logException(Logging.Category.GUI_VIEW, "StatusFragment.loadLayout error: ", e)
                                    }
                                    if (suspendDueToScreenOn) {
                                        statusImage.setImageResource(R.drawable.ic_baseline_stay_current_portrait)
                                        statusHeader.visibility = View.GONE
                                    }
                                    statusDescriptor.text = BOINCActivity.monitor!!.currentStatusDescription
                                }
                                SUSPEND_REASON_USER_REQ -> {
                                    // state after user stops and restarts computation
                                    centerWrapper.visibility = View.GONE
                                    restartingWrapper.visibility = View.VISIBLE
                                }
                                SUSPEND_REASON_BENCHMARKS -> {
                                    statusDescriptor.text = BOINCActivity.monitor!!.currentStatusDescription
                                    statusImage.setImageResource(R.drawable.ic_baseline_timer)
                                    statusHeader.visibility = View.GONE
                                }
                                SUSPEND_REASON_BATTERY_CHARGING -> {
                                    statusDescriptor.text = BOINCActivity.monitor!!.currentStatusDescription
                                    statusImage.setImageResource(R.drawable.ic_baseline_battery_charging_full)
                                    statusHeader.visibility = View.GONE
                                }
                                SUSPEND_REASON_BATTERY_OVERHEATED -> {
                                    statusDescriptor.text = BOINCActivity.monitor!!.currentStatusDescription
                                    statusImage.setImageResource(R.drawable.ic_baseline_battery_alert)
                                    statusHeader.visibility = View.GONE
                                }
                                else -> statusDescriptor.text = BOINCActivity.monitor!!.currentStatusDescription
                            }
                        }
                        ClientStatus.COMPUTING_STATUS_IDLE -> {
                            statusWrapper.visibility = View.VISIBLE
                            centerWrapper.visibility = View.VISIBLE
                            statusHeader.text = BOINCActivity.monitor!!.currentStatusTitle
                            statusHeader.visibility = View.VISIBLE
                            statusImage.setImageResource(R.drawable.ic_baseline_pause)
                            statusImage.contentDescription = BOINCActivity.monitor!!.currentStatusTitle
                            statusImage.isClickable = false
                            statusDescriptor.text = BOINCActivity.monitor!!.currentStatusDescription
                        }
                        ClientStatus.COMPUTING_STATUS_COMPUTING -> statusWrapper.visibility = View.GONE
                    }
                    //save new computing status
                    computingStatus = currentComputingStatus
                    computingSuspendReason = currentComputingSuspendReason
                    networkSuspendReason = currentNetworkSuspendReason
                    setupStatus = -1 // invalidate to force update next time no project
                }
            } else if (currentSetupStatus == ClientStatus.SETUP_STATUS_NOPROJECT) {
                if (setupStatus != ClientStatus.SETUP_STATUS_NOPROJECT) {
                    // set layout and retrieve elements
                    val statusWrapper = requireView().findViewById<LinearLayout>(R.id.status_wrapper)
                    val centerWrapper = requireView().findViewById<LinearLayout>(R.id.center_wrapper)
                    val restartingWrapper = requireView().findViewById<LinearLayout>(R.id.restarting_wrapper)
                    val statusHeader = requireView().findViewById<TextView>(R.id.status_header)
                    val statusImage = requireView().findViewById<ImageView>(R.id.status_image)
                    val statusDescriptor = requireView().findViewById<TextView>(R.id.status_long)
                    statusWrapper.visibility = View.VISIBLE
                    restartingWrapper.visibility = View.GONE
                    centerWrapper.visibility = View.VISIBLE
                    centerWrapper.setOnClickListener(addProjectOnClickListener)
                    statusImage.setImageResource(R.drawable.ic_projects)
                    statusHeader.visibility = View.GONE
                    statusDescriptor.text = BOINCActivity.monitor!!.currentStatusTitle
                    setupStatus = ClientStatus.SETUP_STATUS_NOPROJECT
                    computingStatus = -1
                }
            } else { // BOINC client is not available
                //invalid computingStatus, forces layout on next event
                setupStatus = -1
                computingStatus = -1
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.GUI_VIEW, "StatusFragment.loadLayout error: ", e)
        }
    }

    private val runModeOnClickListener = View.OnClickListener {
        lifecycleScope.launch {
            writeClientMode(RUN_MODE_AUTO)
        }
    }
    private val addProjectOnClickListener = View.OnClickListener {
        startActivity(Intent(activity, SelectionListActivity::class.java))
    }

    private fun writeClientMode(mode: Int) {
        val success = writeClientModeAsync(mode)

        if (success) {
            try {
                BOINCActivity.monitor!!.forceRefresh()
            } catch (e: RemoteException) {
                Logging.logException(Logging.Category.GUI_VIEW, "StatusFragment.writeClientMode() error: ", e)
            }
        } else{
            Logging.logError(Logging.Category.GUI_VIEW, "StatusFragment: setting run and network mode failed")
        }
    }
}
