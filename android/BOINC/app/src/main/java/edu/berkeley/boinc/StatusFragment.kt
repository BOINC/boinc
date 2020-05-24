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
import android.os.RemoteException
import android.util.Log
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
import edu.berkeley.boinc.utils.*
import kotlinx.coroutines.launch

class StatusFragment : Fragment() {
    // keep computingStatus and suspend reason to only adapt layout when changes occur
    private var computingStatus = -1
    private var computingSuspendReason = -1
    private var networkSuspendReason = -1
    private var setupStatus = -1
    private val mClientStatusChangeRec: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            if (Logging.VERBOSE) {
                Log.d(Logging.TAG, "StatusFragment ClientStatusChange - onReceive()")
            }
            loadLayout()
        }
    }
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")
    override fun onResume() {
        //register noisy clientStatusChangeReceiver here, so only active when Activity is visible
        if (Logging.VERBOSE) {
            Log.v(Logging.TAG, "StatusFragment register receiver")
        }
        activity!!.registerReceiver(mClientStatusChangeRec, ifcsc)
        super.onResume()
    }

    override fun onPause() {
        //unregister receiver, so there are not multiple intents flying in
        if (Logging.VERBOSE) {
            Log.v(Logging.TAG, "StatusFragment remove receiver")
        }
        activity!!.unregisterReceiver(mClientStatusChangeRec)
        super.onPause()
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (Logging.VERBOSE) {
            Log.v(Logging.TAG, "StatusFragment onCreateView")
        }
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
                    val statusWrapper = view!!.findViewById<LinearLayout>(R.id.status_wrapper)
                    val centerWrapper = view!!.findViewById<LinearLayout>(R.id.center_wrapper)
                    val restartingWrapper = view!!.findViewById<LinearLayout>(R.id.restarting_wrapper)
                    val statusHeader = view!!.findViewById<TextView>(R.id.status_header)
                    val statusImage = view!!.findViewById<ImageView>(R.id.status_image)
                    val statusDescriptor = view!!.findViewById<TextView>(R.id.status_long)
                    restartingWrapper.visibility = View.GONE
                    when (currentComputingStatus) {
                        ClientStatus.COMPUTING_STATUS_NEVER -> {
                            statusWrapper.visibility = View.VISIBLE
                            statusHeader.text = BOINCActivity.monitor!!.currentStatusTitle
                            statusHeader.visibility = View.VISIBLE
                            statusImage.setImageResource(R.drawable.ic_baseline_play_arrow_black)
                            statusImage.contentDescription = BOINCActivity.monitor!!.currentStatusTitle
                            statusDescriptor.text = BOINCActivity.monitor!!.currentStatusDescription
                            centerWrapper.visibility = View.VISIBLE
                            centerWrapper.setOnClickListener(runModeOnClickListener)
                        }
                        ClientStatus.COMPUTING_STATUS_SUSPENDED -> {
                            statusWrapper.visibility = View.VISIBLE
                            statusHeader.text = BOINCActivity.monitor!!.currentStatusTitle
                            statusHeader.visibility = View.VISIBLE
                            statusImage.setImageResource(R.drawable.ic_baseline_pause_black)
                            statusImage.contentDescription = BOINCActivity.monitor!!.currentStatusTitle
                            statusImage.isClickable = false
                            centerWrapper.visibility = View.VISIBLE
                            when (currentComputingSuspendReason) {
                                SUSPEND_REASON_BATTERIES -> {
                                    statusDescriptor.text = BOINCActivity.monitor!!.currentStatusDescription
                                    statusImage.setImageResource(R.drawable.ic_baseline_power_off_black)
                                    statusHeader.visibility = View.GONE
                                }
                                SUSPEND_REASON_USER_ACTIVE -> {
                                    var suspendDueToScreenOn = false
                                    try {
                                        suspendDueToScreenOn = BOINCActivity.monitor!!.suspendWhenScreenOn
                                    } catch (e: RemoteException) {
                                        if (Logging.ERROR) {
                                            Log.e(Logging.TAG, "StatusFragment.loadLayout error: ", e)
                                        }
                                    }
                                    if (suspendDueToScreenOn) {
                                        statusImage.setImageResource(R.drawable.ic_baseline_stay_current_portrait_black)
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
                                    statusImage.setImageResource(R.drawable.ic_baseline_timer_black)
                                    statusHeader.visibility = View.GONE
                                }
                                SUSPEND_REASON_BATTERY_CHARGING -> {
                                    statusDescriptor.text = BOINCActivity.monitor!!.currentStatusDescription
                                    statusImage.setImageResource(R.drawable.ic_baseline_battery_charging_full_black)
                                    statusHeader.visibility = View.GONE
                                }
                                SUSPEND_REASON_BATTERY_OVERHEATED -> {
                                    statusDescriptor.text = BOINCActivity.monitor!!.currentStatusDescription
                                    statusImage.setImageResource(R.drawable.ic_baseline_battery_alert_black)
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
                            statusImage.setImageResource(R.drawable.ic_baseline_pause_black)
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
                    val statusWrapper = view!!.findViewById<LinearLayout>(R.id.status_wrapper)
                    val centerWrapper = view!!.findViewById<LinearLayout>(R.id.center_wrapper)
                    val restartingWrapper = view!!.findViewById<LinearLayout>(R.id.restarting_wrapper)
                    val statusHeader = view!!.findViewById<TextView>(R.id.status_header)
                    val statusImage = view!!.findViewById<ImageView>(R.id.status_image)
                    val statusDescriptor = view!!.findViewById<TextView>(R.id.status_long)
                    statusWrapper.visibility = View.VISIBLE
                    restartingWrapper.visibility = View.GONE
                    centerWrapper.visibility = View.VISIBLE
                    centerWrapper.setOnClickListener(addProjectOnClickListener)
                    statusImage.setImageResource(R.drawable.ic_projects_black)
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
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "StatusFragment.loadLayout error: ", e)
            }
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

    private suspend fun writeClientMode(mode: Int) {
        val success = writeClientModeAsync(mode)

        if (success) {
            try {
                BOINCActivity.monitor!!.forceRefresh()
            } catch (e: RemoteException) {
                if (Logging.ERROR) {
                    Log.e(Logging.TAG, "StatusFragment.writeClientMode() error: ", e)
                }
            }
        } else if (Logging.WARNING) {
            Log.w(Logging.TAG, "StatusFragment: setting run and network mode failed")
        }
    }
}
