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
package edu.berkeley.boinc.attach.callbacks

import android.net.ConnectivityManager.NetworkCallback
import android.net.Network
import android.net.NetworkInfo
import android.os.Build
import androidx.annotation.RequiresApi

/**
 * Extending ConnectivityManager.NetworkCallback is the recommended mechanism of
 * detecting any network changes, replacing the ConnectivityManager methods that
 * return [NetworkInfo].
 *
 * Since the minimum API level for the app is 19 (KitKat), the old mechanism is used
 * on KitKat, as ConnectivityManager.NetworkCallback is only compatible with API level
 * 21 (Lollipop) and higher.
 */
@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
class BOINCNetworkCallback : NetworkCallback() {
    var isOnline = false
        private set

    override fun onAvailable(network: Network) {
        isOnline = true
    }

    override fun onLost(network: Network) {
        isOnline = false
    }

    override fun onUnavailable() {
        isOnline = false
    }
}
