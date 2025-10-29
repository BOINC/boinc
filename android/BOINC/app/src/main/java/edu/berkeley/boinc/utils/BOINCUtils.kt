/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2025 University of California
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
package edu.berkeley.boinc.utils

import android.content.Context
import android.graphics.Bitmap
import android.net.ConnectivityManager
import android.os.Build
import android.util.Log
import androidx.annotation.ColorRes
import androidx.annotation.DrawableRes
import androidx.appcompat.app.AppCompatDelegate
import androidx.appcompat.content.res.AppCompatResources
import androidx.core.content.ContextCompat
import androidx.core.graphics.drawable.toBitmap
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.R
import java.io.IOException
import java.io.Reader
import java.time.Instant
import java.time.LocalDateTime
import java.time.ZoneId
import java.util.concurrent.Callable
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.async
import kotlinx.coroutines.runBlocking

val ConnectivityManager.isOnline: Boolean
    get() {
        return if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            @Suppress("DEPRECATION")
            activeNetworkInfo?.isConnectedOrConnecting == true
        } else {
            activeNetwork != null
        }
    }

fun setAppTheme(theme: String) {
    when (theme) {
        "light" -> AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_NO)
        "dark" -> AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_YES)
        "system" -> AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM)
    }
}

fun writeClientModeAsync(mode: Int): Boolean {
    val runMode = BOINCActivity.monitor!!.setRunModeAsync(mode)
    val networkMode = BOINCActivity.monitor!!.setNetworkModeAsync(mode)
    return runMode.await() && networkMode.await()
}

//from https://stackoverflow.com/questions/33696488/getting-bitmap-from-vector-drawable
fun Context.getBitmapFromVectorDrawable(@DrawableRes drawableId: Int): Bitmap {
    val drawable = AppCompatResources.getDrawable(this, drawableId)!!
    return drawable.toBitmap()
}

@Throws(IOException::class)
fun Reader.readLineLimit(limit: Int): String? {
    val sb = StringBuilder()
    for (i in 0 until limit) {
        val c = read() //Read in single character
        if (c == -1) {
            return if (sb.isNotEmpty()) sb.toString() else null
        }
        if (c.toChar() == '\n' || c.toChar() == '\r') { //Found end of line, break loop.
            break
        }
        sb.append(c.toChar()) // String is not over and end line not found
    }
    return sb.toString() //end of line was found.
}

fun Context.translateRPCReason(reason: Int) = when (reason) {
    RPC_REASON_USER_REQ -> resources.getString(R.string.rpcreason_userreq)
    RPC_REASON_NEED_WORK -> resources.getString(R.string.rpcreason_needwork)
    RPC_REASON_RESULTS_DUE -> resources.getString(R.string.rpcreason_resultsdue)
    RPC_REASON_TRICKLE_UP -> resources.getString(R.string.rpcreason_trickleup)
    RPC_REASON_ACCT_MGR_REQ -> resources.getString(R.string.rpcreason_acctmgrreq)
    RPC_REASON_INIT -> resources.getString(R.string.rpcreason_init)
    RPC_REASON_PROJECT_REQ -> resources.getString(R.string.rpcreason_projectreq)
    else -> resources.getString(R.string.rpcreason_unknown)
}

fun Long.secondsToLocalDateTime(
        zoneId: ZoneId = ZoneId.systemDefault()
): LocalDateTime = LocalDateTime.ofInstant(Instant.ofEpochSecond(this), zoneId)

fun Context.getColorCompat(@ColorRes colorId: Int) = ContextCompat.getColor(this, colorId)

class TaskRunner<V>(private val callback: ((V) -> Unit)?, private val callable: Callable<V>) {
    val applicationScope = CoroutineScope(SupervisorJob() + Dispatchers.Default)
    private val deferred =  applicationScope.async {
        try {
            val result = callable.call()
            callback?.invoke(result)
            result
        } catch (e: Exception) {
            Logging.logException(Logging.Category.CLIENT, "BOINCUtils.TaskRunner error: ", e)

            throw e
        }
    }

    fun await(): V = runBlocking {
        deferred.await()
    }
}
