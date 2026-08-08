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
package edu.berkeley.boinc.rpc

import java.io.Serializable

data class CcStatus(
        /**
         * Current mode - always/auto/never
         */
        var taskMode: Int = -1,
        /**
         * Permanent version of mode - always/auto/never
         */
        var taskModePerm: Int = -1,
        /**
         * time until task_mode_perm becomes actual task_mode
         */
        var taskModeDelay: Double = 0.0,
        var taskSuspendReason: Int = -1,
        var networkMode: Int = -1,
        var networkModePerm: Int = -1,
        var networkModeDelay: Double = 0.0,
        var networkSuspendReason: Int = -1,
        var networkStatus: Int = -1,
        var amsPasswordError: Boolean = false,
        var managerMustQuit: Boolean = false,
        var disallowAttach: Boolean = false,
        var simpleGuiOnly: Boolean = false
) : Serializable {
    object Fields {
        const val TASK_MODE = "task_mode"
        const val TASK_MODE_PERM = "task_mode_perm"
        const val TASK_MODE_DELAY = "task_mode_delay"
        const val TASK_SUSPEND_REASON = "task_suspend_reason"
        const val NETWORK_MODE = "network_mode"
        const val NETWORK_MODE_PERM = "network_mode_perm"
        const val NETWORK_MODE_DELAY = "network_mode_delay"
        const val NETWORK_SUSPEND_REASON = "network_suspend_reason"
        const val NETWORK_STATUS = "network_status"
        const val AMS_PASSWORD_ERROR = "ams_password_error"
        const val MANAGER_MUST_QUIT = "manager_must_quit"
        const val DISALLOW_ATTACH = "disallow_attach"
        const val SIMPLE_GUI_ONLY = "simple_gui_only"
    }

    companion object {
        private const val serialVersionUID = 1L
    }
}
