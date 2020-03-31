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

import android.content.Context
import android.util.Log
import edu.berkeley.boinc.R
import edu.berkeley.boinc.utils.Logging

open class PrefsListItemWrapper(var context: Context, var id: Int, val isCategory: Boolean) {
    enum class DialogButtonType {
        SLIDER, NUMBER, TEXT
    }

    var dialogButtonType: DialogButtonType? = null
    var header: String? = null
    var description = ""

    // Constructor for elements
    constructor(context: Context, id: Int) : this(context, id, false) {
        header = context.getString(id)
        if (id == R.string.prefs_power_source_header) {
            description = context.getString(R.string.prefs_power_source_description)
        }
        // further description mapping see PrefsListItemWrapperNumber class
    }
}

class PrefsListItemWrapperBool(context: Context?, id: Int?, var status: Boolean)
    : PrefsListItemWrapper(context!!, id!!)

class PrefsListItemWrapperNumber(
        context: Context?,
        id: Int,
        var status: Double,
        dialogButtonType: DialogButtonType?
) : PrefsListItemWrapper(context!!, id) {
    enum class Unit {
        NONE, PERCENT, SECONDS, CELSIUS, MEGABYTES, GIGABYTES, DECIMAL
    }
    var unit: Unit? = null

    init {
        super.dialogButtonType = dialogButtonType
        mapStrings(id)
    }

    private fun mapStrings(id: Int) {
        when (id) {
            R.string.battery_charge_min_pct_header -> {
                description = context.getString(R.string.battery_charge_min_pct_description)
                unit = Unit.PERCENT
            }
            R.string.battery_temperature_max_header -> {
                description = context.getString(R.string.battery_temperature_max_description)
                unit = Unit.CELSIUS
            }
            R.string.prefs_disk_max_pct_header -> {
                description = context.getString(R.string.prefs_disk_max_pct_description)
                unit = Unit.PERCENT
            }
            R.string.prefs_disk_min_free_gb_header -> {
                description = context.getString(R.string.prefs_disk_min_free_gb_description)
                unit = Unit.GIGABYTES
            }
            R.string.prefs_disk_access_interval_header -> {
                description = context.getString(R.string.prefs_disk_access_interval_description)
                unit = Unit.SECONDS
            }
            R.string.prefs_network_daily_xfer_limit_mb_header -> {
                description = context.getString(R.string.prefs_network_daily_xfer_limit_mb_description)
                unit = Unit.MEGABYTES
            }
            R.string.prefs_cpu_number_cpus_header -> {
                description = context.getString(R.string.prefs_cpu_number_cpus_description)
                unit = Unit.NONE
            }
            R.string.prefs_cpu_other_load_suspension_header -> {
                description = context.getString(R.string.prefs_cpu_other_load_suspension_description)
                unit = Unit.PERCENT
            }
            R.string.prefs_cpu_time_max_header -> {
                description = context.getString(R.string.prefs_cpu_time_max_description)
                unit = Unit.PERCENT
            }
            R.string.prefs_memory_max_idle_header -> {
                description = context.getString(R.string.prefs_memory_max_idle_description)
                unit = Unit.PERCENT
            }
            R.string.prefs_other_store_at_least_x_days_of_work_header -> {
                description = context.getString(R.string.prefs_other_store_at_least_x_days_of_work_description)
                unit = Unit.DECIMAL
            }
            R.string.prefs_other_store_up_to_an_additional_x_days_of_work_header -> {
                description = context.getString(R.string.prefs_other_store_up_to_an_additional_x_days_of_work_description)
                unit = Unit.DECIMAL
            }
            R.string.prefs_gui_log_level_header -> {
                description = context.getString(R.string.prefs_gui_log_level_description)
                unit = Unit.NONE
            }
            else -> if (Logging.DEBUG) {
                Log.d(Logging.TAG, "PrefsListItemWrapperNumber map failed!")
            }
        }
    }
}

class PrefsListItemWrapperText(context: Context?, id: Int, var status: String)
    : PrefsListItemWrapper(context!!, id) {
    init {
        dialogButtonType = DialogButtonType.TEXT
        mapStrings(id)
    }

    private fun mapStrings(id: Int) {
        when (id) {
            R.string.prefs_general_device_name_header -> description = ""
            else -> if (Logging.DEBUG) {
                Log.d(Logging.TAG, "PrefsListItemWrapperText map failed!")
            }
        }
    }
}
