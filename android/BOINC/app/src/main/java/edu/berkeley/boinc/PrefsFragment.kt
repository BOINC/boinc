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

import android.app.Dialog
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
import android.view.Window
import android.widget.*
import android.widget.SeekBar.OnSeekBarChangeListener
import androidx.core.content.edit
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import edu.berkeley.boinc.adapter.*
import edu.berkeley.boinc.rpc.GlobalPreferences
import edu.berkeley.boinc.rpc.HostInfo
import edu.berkeley.boinc.utils.Logging
import edu.berkeley.boinc.utils.applyTheme
import kotlinx.coroutines.async
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import java.text.NumberFormat
import java.util.*

class PrefsFragment : Fragment() {
    private lateinit var lv: ListView
    private lateinit var listAdapter: PrefsListAdapter

    // Data for the PrefsListAdapter. This is should be HashMap!
    private val data = ArrayList<PrefsListItemWrapper>()

    // Android specific preferences of the client, read on every onResume via RPC
    private var clientPrefs: GlobalPreferences? = null
    private var hostinfo: HostInfo? = null
    private var layoutSuccessful = false
    private val mClientStatusChangeRec: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            if (Logging.VERBOSE) {
                Log.d(Logging.TAG, "PrefsFragment ClientStatusChange - onReceive()")
            }
            try {
                if (!layoutSuccessful) {
                    populateLayout()
                }
            } catch (e: RemoteException) {
                if (Logging.ERROR) {
                    Log.e(Logging.TAG, "PrefsFragment.BroadcastReceiver: onReceive() error: ", e)
                }
            }
        }
    }
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")

    // fragment lifecycle: 2.
    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (Logging.VERBOSE) {
            Log.d(Logging.TAG, "ProjectsFragment onCreateView")
        }

        // Inflate the layout for this fragment
        val layout = inflater.inflate(R.layout.prefs_layout, container, false)
        lv = layout.findViewById(R.id.listview)
        listAdapter = PrefsListAdapter(activity, this, R.id.listview, data)
        lv.adapter = listAdapter
        return layout
    }

    // fragment lifecycle: 3.
    override fun onResume() {
        try {
            populateLayout()
        } catch (e: RemoteException) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "PrefsFragment.onResume error: ", e)
            }
        }
        activity?.registerReceiver(mClientStatusChangeRec, ifcsc)
        super.onResume()
    }

    override fun onPause() {
        activity?.unregisterReceiver(mClientStatusChangeRec)
        super.onPause()
    }

    // Read preferences from client via rpc
    private val prefs: Boolean
        get() {
            // Try to get current client status from monitor
            clientPrefs = try {
                BOINCActivity.monitor!!.prefs // Read preferences from client via rpc
            } catch (e: Exception) {
                if (Logging.WARNING) {
                    Log.w(Logging.TAG, "PrefsActivity: Could not load data, clientStatus not initialized.")
                }
                e.printStackTrace()
                return false
            }
            if (clientPrefs == null) {
                if (Logging.DEBUG) {
                    Log.d(Logging.TAG, "readPrefs: null, return false")
                }
                return false
            }
            return true
        }

    // Get the hostinfo from client via RPC
    private val hostInfo: Boolean
        get() {
            // Try to get current client status from monitor
            hostinfo = try {
                BOINCActivity.monitor!!.hostInfo // Get the hostinfo from client via rpc
            } catch (e: Exception) {
                if (Logging.WARNING) {
                    Log.w(Logging.TAG, "PrefsActivity: Could not load data, clientStatus not initialized.")
                }
                e.printStackTrace()
                return false
            }
            if (hostinfo == null) {
                if (Logging.DEBUG) {
                    Log.d(Logging.TAG, "getHostInfo: null, return false")
                }
                return false
            }
            return true
        }

    @Throws(RemoteException::class)
    private fun populateLayout() {
        if (!prefs || BOINCActivity.monitor == null || !hostInfo) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "PrefsFragment.populateLayout returns, data is not present")
            }
            return
        }
        data.clear()
        val advanced = BOINCActivity.monitor!!.showAdvanced
        val stationaryDeviceMode = BOINCActivity.monitor!!.stationaryDeviceMode
        val stationaryDeviceSuspected = BOINCActivity.monitor!!.isStationaryDeviceSuspected

        // The order is important, the GUI will be displayed in the same order as the data is added.
        // General
        data.add(PrefsListItemWrapper(activity!!, R.string.prefs_category_general, true))
        data.add(PrefsListItemWrapperBool(activity!!, R.string.prefs_autostart_header,
                BOINCActivity.monitor!!.autostart))
        data.add(PrefsListItemWrapperBool(activity!!, R.string.prefs_show_notification_notices_header,
                BOINCActivity.monitor!!.showNotificationForNotices))
        data.add(PrefsListItemWrapperBool(activity!!, R.string.prefs_show_advanced_header,
                BOINCActivity.monitor!!.showAdvanced))
        if (!stationaryDeviceMode) {
            data.add(PrefsListItemWrapperBool(activity!!, R.string.prefs_suspend_when_screen_on,
                    BOINCActivity.monitor!!.suspendWhenScreenOn))
        }
        data.add(PrefsListItemWrapperText(activity!!, R.string.prefs_general_device_name_header,
                BOINCActivity.monitor!!.hostInfo.domainName!!))

        val sharedPrefs = requireContext().getSharedPreferences("PREFS", 0)
        val darkTheme = sharedPrefs.getBoolean("darkTheme", false)
        data.add(PrefsListItemWrapperBool(requireActivity(), R.string.prefs_dark_theme, darkTheme))

        // Network
        data.add(PrefsListItemWrapper(activity!!, R.string.prefs_category_network, true))
        data.add(PrefsListItemWrapperBool(activity!!, R.string.prefs_network_wifi_only_header,
                clientPrefs!!.networkWiFiOnly))
        if (advanced) {
            data.add(PrefsListItemWrapperNumber(activity!!,
                    R.string.prefs_network_daily_xfer_limit_mb_header, clientPrefs!!.dailyTransferLimitMB,
                    PrefsListItemWrapper.DialogButtonType.NUMBER))
        }

        // Power
        data.add(PrefsListItemWrapper(activity!!, R.string.prefs_category_power, true))
        if (stationaryDeviceSuspected) { // API indicates that there is no battery, offer opt-in preference for stationary device mode
            data.add(PrefsListItemWrapperBool(activity!!,
                    R.string.prefs_stationary_device_mode_header,
                    BOINCActivity.monitor!!.stationaryDeviceMode))
        }
        if (!stationaryDeviceMode) { // Client would compute regardless of battery preferences, so only show if that is not the case
            data.add(PrefsListItemWrapper(activity!!, R.string.prefs_power_source_header))
            data.add(PrefsListItemWrapperNumber(activity!!, R.string.battery_charge_min_pct_header,
                    clientPrefs!!.batteryChargeMinPct,
                    PrefsListItemWrapper.DialogButtonType.SLIDER))
            if (advanced) {
                data.add(PrefsListItemWrapperNumber(activity!!, R.string.battery_temperature_max_header,
                        clientPrefs!!.batteryMaxTemperature,
                        PrefsListItemWrapper.DialogButtonType.NUMBER))
            }
        }
        if (advanced) {
            // CPU
            data.add(PrefsListItemWrapper(activity!!, R.string.prefs_category_cpu, true))
            if (hostinfo!!.noOfCPUs > 1) {
                data.add(PrefsListItemWrapperNumber(activity!!,
                        R.string.prefs_cpu_number_cpus_header,
                        pctCpuCoresToNumber(clientPrefs!!.maxNoOfCPUsPct),
                        PrefsListItemWrapper.DialogButtonType.SLIDER))
            }
            data.add(PrefsListItemWrapperNumber(activity!!, R.string.prefs_cpu_time_max_header,
                    clientPrefs!!.cpuUsageLimit,
                    PrefsListItemWrapper.DialogButtonType.SLIDER))
            data.add(PrefsListItemWrapperNumber(activity!!, R.string.prefs_cpu_other_load_suspension_header,
                    clientPrefs!!.suspendCpuUsage,
                    PrefsListItemWrapper.DialogButtonType.SLIDER))

            // Storage
            data.add(PrefsListItemWrapper(activity!!, R.string.prefs_category_storage, true))
            data.add(PrefsListItemWrapperNumber(activity!!, R.string.prefs_disk_max_pct_header,
                    clientPrefs!!.diskMaxUsedPct,
                    PrefsListItemWrapper.DialogButtonType.SLIDER))
            data.add(PrefsListItemWrapperNumber(activity!!, R.string.prefs_disk_min_free_gb_header,
                    clientPrefs!!.diskMinFreeGB,
                    PrefsListItemWrapper.DialogButtonType.NUMBER))
            data.add(PrefsListItemWrapperNumber(activity!!, R.string.prefs_disk_access_interval_header,
                    clientPrefs!!.diskInterval,
                    PrefsListItemWrapper.DialogButtonType.NUMBER))

            // Memory
            data.add(PrefsListItemWrapper(activity!!, R.string.prefs_category_memory,
                    true))
            data.add(PrefsListItemWrapperNumber(activity!!,
                    R.string.prefs_memory_max_idle_header,
                    clientPrefs!!.ramMaxUsedIdleFrac,
                    PrefsListItemWrapper.DialogButtonType.SLIDER))

            // Other
            data.add(PrefsListItemWrapper(activity!!, R.string.prefs_category_other, true))
            data.add(PrefsListItemWrapperNumber(activity!!,
                    R.string.prefs_other_store_at_least_x_days_of_work_header,
                    clientPrefs!!.workBufMinDays,
                    PrefsListItemWrapper.DialogButtonType.NUMBER))
            data.add(PrefsListItemWrapperNumber(activity!!,
                    R.string.prefs_other_store_up_to_an_additional_x_days_of_work_header,
                    clientPrefs!!.workBufAdditionalDays,
                    PrefsListItemWrapper.DialogButtonType.NUMBER))

            // Debug
            data.add(PrefsListItemWrapper(activity!!, R.string.prefs_category_debug,
                    true))
            data.add(PrefsListItemWrapper(activity!!, R.string.prefs_client_log_flags_header))
            data.add(PrefsListItemWrapperNumber(activity!!, R.string.prefs_gui_log_level_header,
                    BOINCActivity.monitor!!.logLevel.toDouble(),
                    PrefsListItemWrapper.DialogButtonType.SLIDER))
        }
        updateLayout()
        layoutSuccessful = true
    }

    private fun updateLayout() {
        listAdapter.notifyDataSetChanged()
    }

    // Updates list item of boolean preference
    // Requires updateLayout to be called afterwards
    private fun updateBoolPreference(id: Int, newValue: Boolean) {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "updateBoolPreference for ID: $id $VALUE_LOG $newValue")
        }
        val first = data.firstOrNull { it.id == id }
        if (first != null) {
            (first as PrefsListItemWrapperBool).status = newValue
        }
    }

    // Updates list item of number preference
    // Requires updateLayout to be called afterwards
    private fun updateNumberPreference(id: Int, newValue: Double) {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "updateNumberPreference for ID: $id $VALUE_LOG $newValue")
        }
        val first = data.firstOrNull { it.id == id }
        if (first != null) {
            (first as PrefsListItemWrapperNumber).status = newValue
        }
    }

    // Updates list item of text preference
    private fun updateTextPreference(id: Int, newValue: String) {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "updateTextPreference for ID: $id $VALUE_LOG $newValue")
        }
        val first = data.firstOrNull { it.id == id }
        if (first != null) {
            (first as PrefsListItemWrapperText).status = newValue
        }
    }

    private fun setupSliderDialog(item: PrefsListItemWrapper, dialog: Dialog) {
        val prefsListItemWrapperNumber = item as PrefsListItemWrapperNumber
        dialog.setContentView(R.layout.prefs_layout_dialog_pct)
        val slider = dialog.findViewById<SeekBar>(R.id.seekbar)
        if (prefsListItemWrapperNumber.id == R.string.battery_charge_min_pct_header ||
                prefsListItemWrapperNumber.id == R.string.prefs_disk_max_pct_header ||
                prefsListItemWrapperNumber.id == R.string.prefs_cpu_time_max_header ||
                prefsListItemWrapperNumber.id == R.string.prefs_cpu_other_load_suspension_header ||
                prefsListItemWrapperNumber.id == R.string.prefs_memory_max_idle_header) {
            val seekBarDefault = prefsListItemWrapperNumber.status / 10
            slider.progress = seekBarDefault.toInt()
            val onSeekBarChangeListener: OnSeekBarChangeListener = object : OnSeekBarChangeListener {
                override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                    val progressString = NumberFormat.getPercentInstance().format(progress / 10.0)
                    val sliderProgress = dialog.findViewById<TextView>(R.id.seekbar_status)
                    sliderProgress.text = progressString
                }

                override fun onStartTrackingTouch(seekBar: SeekBar) {}
                override fun onStopTrackingTouch(seekBar: SeekBar) {}
            }
            slider.setOnSeekBarChangeListener(onSeekBarChangeListener)
            onSeekBarChangeListener.onProgressChanged(slider, seekBarDefault.toInt(), false)
        } else if (prefsListItemWrapperNumber.id == R.string.prefs_cpu_number_cpus_header) {
            if (!hostInfo) {
                if (Logging.WARNING) {
                    Log.w(Logging.TAG, "onItemClick missing hostInfo")
                }
                return
            }
            slider.max = if (hostinfo!!.noOfCPUs <= 1) 0 else hostinfo!!.noOfCPUs - 1
            val statusValue = prefsListItemWrapperNumber.status.toInt()
            slider.progress = if (statusValue <= 0) 0 else (statusValue - 1).coerceAtMost(slider.max)
            Log.d(Logging.TAG, String.format("statusValue == %,d", statusValue))
            val onSeekBarChangeListener: OnSeekBarChangeListener = object : OnSeekBarChangeListener {
                override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                    val progressString = NumberFormat.getIntegerInstance().format(
                            if (progress <= 0) 1 else progress + 1.toLong()) // do not allow 0 cpus
                    val sliderProgress = dialog.findViewById<TextView>(R.id.seekbar_status)
                    sliderProgress.text = progressString
                }

                override fun onStartTrackingTouch(seekBar: SeekBar) {}
                override fun onStopTrackingTouch(seekBar: SeekBar) {}
            }
            slider.setOnSeekBarChangeListener(onSeekBarChangeListener)
            onSeekBarChangeListener.onProgressChanged(slider, statusValue - 1, false)
        } else if (prefsListItemWrapperNumber.id == R.string.prefs_gui_log_level_header) {
            slider.max = 5
            slider.progress = prefsListItemWrapperNumber.status.toInt()
            val onSeekBarChangeListener: OnSeekBarChangeListener = object : OnSeekBarChangeListener {
                override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                    val progressString = NumberFormat.getIntegerInstance().format(progress.toLong())
                    val sliderProgress = dialog.findViewById<TextView>(R.id.seekbar_status)
                    sliderProgress.text = progressString
                }

                override fun onStartTrackingTouch(seekBar: SeekBar) {}
                override fun onStopTrackingTouch(seekBar: SeekBar) {}
            }
            slider.setOnSeekBarChangeListener(onSeekBarChangeListener)
            onSeekBarChangeListener.onProgressChanged(slider, prefsListItemWrapperNumber.status.toInt(),
                    false)
        }
        setupDialogButtons(item, dialog)
    }

    @Throws(RemoteException::class)
    private fun setupSelectionListDialog(item: PrefsListItemWrapper, dialog: Dialog) {
        dialog.setContentView(R.layout.prefs_layout_dialog_selection)
        if (item.id == R.string.prefs_client_log_flags_header) {
            val array = resources.getStringArray(R.array.prefs_client_log_flags)
            val options = array.map { SelectionDialogOption(it) }.toMutableList()
            val lv = dialog.findViewById<ListView>(R.id.selection)
            PrefsSelectionDialogListAdapter(activity, lv, R.id.selection, options)

            // Setup confirm button action
            val confirm = dialog.findViewById<Button>(R.id.confirm)
            confirm.setOnClickListener {
                val selectedOptions: MutableList<String> = ArrayList()
                for (option in options) {
                    if (option.isSelected) {
                        selectedOptions.add(option.name)
                    }
                }
                if (Logging.DEBUG) {
                    Log.d(Logging.TAG, selectedOptions.size.toString() + " log flags selected")
                }
                lifecycleScope.launch {
                    val ccConfig = formatOptionsToCcConfig(selectedOptions)
                    if (Logging.DEBUG) {
                        Log.d(Logging.TAG, "SetCcConfigAsync with: $ccConfig")
                    }
                    BOINCActivity.monitor!!.setCcConfig(ccConfig)
                }
                dialog.dismiss()
            }
        } else if (item.id == R.string.prefs_power_source_header) {
            val options = listOf(
                    SelectionDialogOption(this, R.string.prefs_power_source_ac,
                            BOINCActivity.monitor!!.powerSourceAc),
                    SelectionDialogOption(this, R.string.prefs_power_source_usb,
                            BOINCActivity.monitor!!.powerSourceUsb),
                    SelectionDialogOption(this, R.string.prefs_power_source_wireless,
                            BOINCActivity.monitor!!.powerSourceWireless),
                    SelectionDialogOption(this, R.string.prefs_power_source_battery,
                            clientPrefs!!.runOnBatteryPower,
                            true)
            )
            val lv = dialog.findViewById<ListView>(R.id.selection)
            PrefsSelectionDialogListAdapter(activity, lv, R.id.selection, options)

            // Setup confirm button action
            dialog.findViewById<Button>(R.id.confirm).setOnClickListener {
                try {
                    for (option in options) {
                        when (option.id) {
                            R.string.prefs_power_source_ac -> BOINCActivity.monitor!!.powerSourceAc = option.isSelected
                            R.string.prefs_power_source_usb -> BOINCActivity.monitor!!.powerSourceUsb = option.isSelected
                            R.string.prefs_power_source_wireless -> BOINCActivity.monitor!!.powerSourceWireless = option.isSelected
                            R.string.prefs_power_source_battery -> {
                                clientPrefs!!.runOnBatteryPower = option.isSelected
                                lifecycleScope.launch {
                                    writeClientPrefs() // coroutine triggers layout update
                                }
                            }
                        }
                    }
                    dialog.dismiss()
                } catch (e: RemoteException) {
                    if (Logging.ERROR) {
                        Log.e(Logging.TAG, "PrefsFragment.setupSelectionListDialog.setOnClickListener: OnClick() error: ", e)
                    }
                }
            }
        }

        // Generic cancel button
        dialog.findViewById<Button>(R.id.cancel).setOnClickListener { dialog.dismiss() }
    }

    private fun setupDialogButtons(item: PrefsListItemWrapper, dialog: Dialog) {
        // Confirm
        val confirm = dialog.findViewById<Button>(R.id.confirm)
        confirm.setOnClickListener {
            // Sliders
            when {
                item.dialogButtonType === PrefsListItemWrapper.DialogButtonType.SLIDER -> {
                    val slider = dialog.findViewById<SeekBar>(R.id.seekbar)
                    val sliderProgress = slider.progress
                    val value: Double

                    // Calculate value based on Slider Progress
                    when (item.id) {
                        R.string.prefs_cpu_number_cpus_header -> {
                            value = numberCpuCoresToPct(if (sliderProgress <= 0) 1.0 else sliderProgress + 1.0)
                            writeClientNumberPreference(item.id, value)
                        }
                        R.string.prefs_gui_log_level_header -> {
                            try {
                                // Monitor and UI in two different processes. set static variable in both
                                Logging.setLogLevel(sliderProgress)
                                BOINCActivity.monitor!!.logLevel = sliderProgress
                            } catch (e: RemoteException) {
                                if (Logging.ERROR) {
                                    Log.e(Logging.TAG,
                                            "PrefsFragment.setupSelectionListDialog.setOnClickListener: OnClick() error: ",
                                            e)
                                }
                            }
                            updateNumberPreference(item.id, sliderProgress.toDouble())
                            updateLayout()
                        }
                        else -> {
                            value = sliderProgress * 10.0
                            writeClientNumberPreference(item.id, value)
                        }
                    }
                }
                item.dialogButtonType === PrefsListItemWrapper.DialogButtonType.NUMBER -> {
                    val edit = dialog.findViewById<EditText>(R.id.Input)
                    val input = edit.text.toString()
                    val valueTmp = parseInputValueToDouble(input) ?: return@setOnClickListener
                    writeClientNumberPreference(item.id, valueTmp)
                }
                item.dialogButtonType === PrefsListItemWrapper.DialogButtonType.TEXT -> {
                    val input = dialog.findViewById<EditText>(R.id.Input)
                    if (item.id == R.string.prefs_general_device_name_header) {
                        try {
                            if (!BOINCActivity.monitor!!.setDomainName(input.text.toString())) {
                                if (Logging.DEBUG) {
                                    Log.d(Logging.TAG, "PrefsFragment.setupDialogButtons.onClick.setDomainName(): false")
                                }
                            }
                        } catch (e: Exception) {
                            if (Logging.ERROR) {
                                Log.e(Logging.TAG, "PrefsFragment.setupDialogButtons.onClick(): error: $e")
                            }
                        }
                    }
                    updateTextPreference(item.id, input.text.toString())
                }
            }
            dialog.dismiss()
        }

        // Cancel
        val cancel = dialog.findViewById<Button>(R.id.cancel)
        cancel.setOnClickListener { dialog.dismiss() }
    }

    private fun writeClientNumberPreference(id: Int, value: Double) {
        // Update preferences
        var value = value
        when (id) {
            R.string.prefs_disk_max_pct_header -> clientPrefs!!.diskMaxUsedPct = value
            R.string.prefs_disk_min_free_gb_header -> clientPrefs!!.diskMinFreeGB = value
            R.string.prefs_disk_access_interval_header -> clientPrefs!!.diskInterval = value
            R.string.prefs_network_daily_xfer_limit_mb_header -> {
                clientPrefs!!.dailyTransferLimitMB = value
                // also need to set the period!
                clientPrefs!!.dailyTransferPeriodDays = 1
            }
            R.string.battery_charge_min_pct_header -> clientPrefs!!.batteryChargeMinPct = value
            R.string.battery_temperature_max_header -> clientPrefs!!.batteryMaxTemperature = value
            R.string.prefs_cpu_number_cpus_header -> {
                clientPrefs!!.maxNoOfCPUsPct = value
                value = pctCpuCoresToNumber(value) // Convert value back to number for layout update
            }
            R.string.prefs_cpu_time_max_header -> clientPrefs!!.cpuUsageLimit = value
            R.string.prefs_cpu_other_load_suspension_header -> clientPrefs!!.suspendCpuUsage = value
            R.string.prefs_memory_max_idle_header -> clientPrefs!!.ramMaxUsedIdleFrac = value
            R.string.prefs_other_store_at_least_x_days_of_work_header -> clientPrefs!!.workBufMinDays = value
            R.string.prefs_other_store_up_to_an_additional_x_days_of_work_header -> clientPrefs!!.workBufAdditionalDays = value
            else -> {
                if (Logging.DEBUG) {
                    Log.d(Logging.TAG, "onClick (dialog submit button), couldnt match ID")
                }
                val toast = Toast.makeText(activity, "ooops! something went wrong...", Toast.LENGTH_SHORT)
                toast.show()
                return
            }
        }
        // Update list item
        updateNumberPreference(id, value)
        // Preferences adapted, write preferences to client
        lifecycleScope.launch {
            writeClientPrefs()
        }
    }

    private fun numberCpuCoresToPct(ncpus: Double): Double {
        val pct = ncpus / hostinfo!!.noOfCPUs.toDouble() * 100
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "numberCpuCoresToPct: " + ncpus + hostinfo!!.noOfCPUs + pct)
        }
        return pct
    }

    private fun pctCpuCoresToNumber(pct: Double): Double {
        return 1.0.coerceAtLeast(hostinfo!!.noOfCPUs.toDouble() * (pct / 100.0))
    }

    private fun parseInputValueToDouble(input: String): Double? {
        // Parse value
        var input = input
        val value: Double
        return try {
            input = input.replace(",", ".") //Replace e.g. European decimal separator "," by "."
            value = input.toDouble()
            if (Logging.DEBUG) {
                Log.d(Logging.TAG, "parseInputValueToDouble: $value")
            }
            value
        } catch (e: Exception) {
            if (Logging.WARNING) {
                Log.w(Logging.TAG, e)
            }
            val toast = Toast.makeText(activity, "wrong format!", Toast.LENGTH_SHORT)
            toast.show()
            null
        }
    }

    private fun formatOptionsToCcConfig(options: List<String>): String {
        val builder = StringBuilder()
        builder.append("<cc_config>\n <log_flags>\n")
        for (option in options) {
            builder.append("  <").append(option).append("/>\n")
        }
        builder.append(" </log_flags>\n <options>\n </options>\n</cc_config>")
        return builder.toString()
    }

    inner class BoolOnClick(private val ID: Int, private val cb: CheckBox) : View.OnClickListener {
        override fun onClick(view: View) {
            if (Logging.DEBUG) {
                Log.d(Logging.TAG, "onCbClick")
            }
            val previousState = cb.isChecked
            cb.isChecked = !previousState
            val isSet = cb.isChecked
            try {
                when (ID) {
                    R.string.prefs_autostart_header -> {
                        BOINCActivity.monitor!!.autostart = isSet
                        updateBoolPreference(ID, isSet)
                        updateLayout()
                    }
                    R.string.prefs_show_notification_notices_header -> {
                        BOINCActivity.monitor!!.showNotificationForNotices = isSet
                        updateBoolPreference(ID, isSet)
                        updateLayout()
                    }
                    R.string.prefs_show_advanced_header -> {
                        BOINCActivity.monitor!!.showAdvanced = isSet
                        // reload complete layout to remove/add advanced elements
                        populateLayout()
                    }
                    R.string.prefs_suspend_when_screen_on -> {
                        BOINCActivity.monitor!!.suspendWhenScreenOn = isSet
                        updateBoolPreference(ID, isSet)
                        updateLayout()
                    }
                    R.string.prefs_dark_theme -> {
                        val sharedPrefs = requireContext().getSharedPreferences("PREFS", 0)
                        sharedPrefs.edit { putBoolean("darkTheme", isSet) }
                        applyTheme(requireContext())
                    }
                    R.string.prefs_network_wifi_only_header -> {
                        clientPrefs!!.networkWiFiOnly = isSet
                        updateBoolPreference(ID, isSet)
                        lifecycleScope.launch {
                            writeClientPrefs() //coroutine triggers layout update
                        }
                    }
                    R.string.prefs_stationary_device_mode_header -> {
                        BOINCActivity.monitor!!.stationaryDeviceMode = isSet
                        // reload complete layout to remove/add power preference elements
                        populateLayout()
                    }
                }
            } catch (e: RemoteException) {
                if (Logging.ERROR) {
                    Log.e(Logging.TAG, "PrefsFragment.BoolOnClick: onClick() error: ", e)
                }
            }
        }

    }

    inner class ValueOnClick(private val item: PrefsListItemWrapper) : View.OnClickListener {
        override fun onClick(view: View) {
            val dialog = Dialog(activity!!)
            dialog.requestWindowFeature(Window.FEATURE_NO_TITLE)
            when (item.id) {
                R.string.prefs_general_device_name_header -> {
                    dialog.setContentView(R.layout.prefs_layout_dialog_text)
                    (dialog.findViewById<TextView>(R.id.pref)).setText(item.id)
                    setupDialogButtons(item, dialog)
                }
                R.string.prefs_network_daily_xfer_limit_mb_header, R.string.battery_temperature_max_header,
                R.string.prefs_disk_min_free_gb_header, R.string.prefs_disk_access_interval_header,
                R.string.prefs_other_store_at_least_x_days_of_work_header,
                R.string.prefs_other_store_up_to_an_additional_x_days_of_work_header -> {
                    dialog.setContentView(R.layout.prefs_layout_dialog_number)
                    (dialog.findViewById<TextView>(R.id.pref)).setText(item.id)
                    setupDialogButtons(item, dialog)
                }
                R.string.prefs_power_source_header, R.string.prefs_client_log_flags_header -> try {
                    setupSelectionListDialog(item, dialog)
                } catch (e: RemoteException) {
                    if (Logging.ERROR) {
                        Log.e(Logging.TAG, "PrefsFragment.ValueOnClick.onClick() error: ", e)
                    }
                }
                R.string.battery_charge_min_pct_header, R.string.prefs_cpu_number_cpus_header,
                R.string.prefs_cpu_time_max_header, R.string.prefs_cpu_other_load_suspension_header,
                R.string.prefs_disk_max_pct_header, R.string.prefs_memory_max_idle_header,
                R.string.prefs_gui_log_level_header -> {
                    setupSliderDialog(item, dialog)
                    (dialog.findViewById<TextView>(R.id.pref)).setText(item.id)
                }
                else -> {
                    if (Logging.ERROR) {
                        Log.d(Logging.TAG, "PrefsActivity onItemClick: could not map ID: " + item.id)
                    }
                    return
                }
            }

            // show dialog
            dialog.show()
        }
    }

    private suspend fun writeClientPrefs() = coroutineScope {
        val success = async {
            return@async try {
                BOINCActivity.monitor!!.setGlobalPreferences(clientPrefs)
            } catch (e: RemoteException) {
                false
            }
        }

        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "writeClientPrefs() async call returned: ${success.await()}")
        }
        updateLayout()
    }

    companion object {
        private const val VALUE_LOG = " value: "
    }
}
