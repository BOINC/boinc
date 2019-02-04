/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2014 University of California
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
package edu.berkeley.boinc.mutex;

import java.io.IOException;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.util.Log;

import edu.berkeley.boinc.utils.Logging;

/**
 * Mediates usage of device for volunteer computing. Acquire this lock before executing computations.
 * Indicates whether device is already in use by another BOINC-based Android application.
 *
 * @author Joachim Fritzsch
 */
public class BoincMutex {

    private static final String boincMutex = "boinc_mutex";
    private LocalSocket socket = new LocalSocket();
    public boolean acquired = false;

    /**
     * Tries to acquire BOINC mutex. Only run computations on device, if this function returned true.
     * Mutex is freed automatically when application closes, to release earlier, call release()
     *
     * @return mutex acquisition successful
     */
    public boolean acquire() {
        if(socket.isBound()) {
            return true;
        }
        try {
            socket.bind(new LocalSocketAddress(boincMutex));
            acquired = true;
        }
        catch(IOException e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "BoincMutex.acquire error: ", e);
            }
        }
        return socket.isBound();
    }

    /**
     * Releases BOINC mutex. Re-acquire mutex before resuming computation.
     */
    public void release() {
        if(socket.isBound()) {
            try {
                socket.close();
                acquired = false;
            }
            catch(IOException e) {
                if(Logging.ERROR) {
                    Log.e(Logging.TAG, "BoincMutex.release error: ", e);
                }
            }
        }
    }


}
