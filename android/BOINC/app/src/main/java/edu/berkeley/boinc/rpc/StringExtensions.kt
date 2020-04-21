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
@file:JvmName("StringExtensions")

package edu.berkeley.boinc.rpc

import android.util.Log
import edu.berkeley.boinc.utils.Logging
import java.nio.charset.StandardCharsets
import java.security.MessageDigest

fun String.equalsAny(vararg strings: String, ignoreCase: Boolean) =
        strings.any { this.equals(it, ignoreCase = ignoreCase) }

/**
 * Returns the MD5 hash of the String.
 */
fun String.hash(): String {
    return try {
        val md5 = MessageDigest.getInstance("MD5")
        md5.update(toByteArray(StandardCharsets.ISO_8859_1), 0, length)
        md5.digest().joinToString(separator = "") { String.format("%02x", it) }
    } catch (e: Exception) {
        if (Logging.WARNING) {
            Log.w(Logging.TAG, "Error when calculating MD5 hash")
        }
        ""
    }
}
