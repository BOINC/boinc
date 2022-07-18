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
package edu.berkeley.boinc.client

import android.os.Build
import android.os.PowerManager
import com.google.common.io.ByteStreams
import com.google.common.io.Files
import java.io.File
import java.io.InputStream

// This file contains extensions that are only used in Monitor.
// This file was created to avoid creating clutter in the Monitor class file.

internal fun allNotNull(vararg values: Any?) = values.none { it == null }

internal fun CharSequence.containsAny(vararg sequences: CharSequence) = sequences.any { it in this }

internal fun InputStream.copyToFile(destFile: File) {
    destFile.parentFile?.mkdirs()
    Files.write(ByteStreams.toByteArray(this), destFile)
}

internal val PowerManager.isScreenOnCompat: Boolean
    get() {
        return if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT_WATCH) {
            @Suppress("DEPRECATION")
            isScreenOn
        } else {
            isInteractive
        }
    }
