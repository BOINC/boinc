/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2012 University of California
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
package edu.berkeley.boinc.utils;

public class Logging {
    private Logging() {}

    public static final String TAG = "BOINC_GUI";

    public static int LOGLEVEL = -1;
    public static Boolean ERROR = false;
    public static Boolean WARNING = false;
    public static Boolean INFO = false;
    public static Boolean DEBUG = false;
    public static Boolean VERBOSE = false;

    public static Boolean RPC_PERFORMANCE = false;
    public static Boolean RPC_DATA = false;

    public static void setLogLevel(Integer logLevel) {
        LOGLEVEL = logLevel;
        ERROR = LOGLEVEL > 0;
        WARNING = LOGLEVEL > 1;
        INFO = LOGLEVEL > 2;
        DEBUG = LOGLEVEL > 3;
        VERBOSE = LOGLEVEL > 4;
    }
}
