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
package edu.berkeley.boinc.mutex

import android.net.LocalSocket
import android.net.LocalSocketAddress
import edu.berkeley.boinc.utils.Logging
import java.io.IOException
import javax.inject.Inject
import javax.inject.Singleton

/**
 * Mediates usage of device for volunteer computing. Acquire this lock before executing computations.
 * Indicates whether device is already in use by another BOINC-based Android application.
 *
 * @author Joachim Fritzsch
 */
@Singleton
class BoincMutex @Inject constructor(private val socket: LocalSocket) {
    var isAcquired = false
        private set

    /**
     * Tries to acquire the BOINC mutex and returns whether the mutex acquisition was successful.
     * Only run computations on device if this function returns `true`.
     *
     * Mutex is freed automatically when application is closed; to release earlier, call [release].
     */
    fun acquire(): Boolean {
        if (socket.isBound) {
            return true
        }
        try {
            socket.bind(LocalSocketAddress("boinc_mutex"))
            isAcquired = true
        } catch (e: IOException) {
            Logging.logException(Logging.Category.CLIENT, "BoincMutex.acquire error: ", e)
        }
        return socket.isBound
    }

    /**
     * Releases the BOINC mutex. Re-acquire mutex before resuming computation.
     */
    fun release() {
        if (socket.isBound) {
            try {
                socket.close()
                isAcquired = false
            } catch (e: IOException) {
                Logging.logException(Logging.Category.CLIENT, "BoincMutex.release error: ", e)
            }
        }
    }
}
