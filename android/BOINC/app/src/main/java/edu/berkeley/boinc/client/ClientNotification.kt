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
package edu.berkeley.boinc.client

import android.annotation.SuppressLint
import android.app.Notification
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service.STOP_FOREGROUND_REMOVE
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.Build.VERSION
import android.os.Build.VERSION_CODES
import androidx.annotation.VisibleForTesting
import androidx.core.app.NotificationCompat
import androidx.core.content.getSystemService
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.Result
import edu.berkeley.boinc.utils.Logging
import edu.berkeley.boinc.utils.getBitmapFromVectorDrawable
import java.util.*
import javax.inject.Inject
import javax.inject.Singleton

@SuppressLint("WrongConstant")
@Singleton
class ClientNotification @Inject constructor(private val context: Context) {
    private val nm = context.getSystemService<NotificationManager>()!!
    private val notificationId: Int = context.resources.getInteger(R.integer.autostart_notification_id)
    private val contentIntent: PendingIntent
    private var n: Notification? = null

    @VisibleForTesting
    internal var mOldComputingStatus = -1

    @VisibleForTesting
    internal var mOldSuspendReason = -1

    @VisibleForTesting
    internal var mOldActiveTasks: MutableList<Result> = ArrayList()

    @VisibleForTesting
    internal var notificationShown = false

    // debug foreground state by running
    // adb shell: dumpsys activity services edu.berkeley.boinc
    @VisibleForTesting
    internal var foreground = false

    /**
     * Updates notification with client's current status. Notifies if not present. Checking
     * notification-related preferences.
     *
     * @param updatedStatus client status data
     * @param service       reference to service, sets to foreground if active
     * @param active        indicator whether BOINC should stay in foreground (during computing and
     * idle, i.e. not suspended)
     */
    fun update(updatedStatus: ClientStatus?, service: Monitor?, active: Boolean) {
        // nop if data is not present
        if (service == null || updatedStatus == null) {
            return
        }

        // stop service foreground, if not active anymore
        if(!active && foreground) {
            setForegroundState(service, false);
        }

        // if not active, check preference whether to show notification during suspension
        if(!active && !service.appPreferences.showNotificationDuringSuspend) {
            // cancel notification if necessary
            if(notificationShown) {
                Logging.logInfo(Logging.Category.CLIENT, "ClientNotification: cancel suspension notification due to preference.");
                nm.cancel(notificationId);
                notificationShown = false;
            }
            return;
        }

        //check if active tasks have changed to force update
        var activeTasksChanged = false
        if (updatedStatus.computingStatus == ClientStatus.COMPUTING_STATUS_COMPUTING) {
            val activeTasks = updatedStatus.executingTasks
            if (activeTasks.size != mOldActiveTasks.size) {
                activeTasksChanged = true
            } else {
                for (x in activeTasks.indices) {
                    if (activeTasks[x].name != mOldActiveTasks[x].name) {
                        activeTasksChanged = true
                        Logging.logVerbose(
                            Logging.Category.TASKS, "Active task: " + activeTasks[x].name +
                                    ", old active task: " +
                                    mOldActiveTasks[x].name
                        )
                        break
                    }
                }
            }
            if (activeTasksChanged) {
                mOldActiveTasks = activeTasks.toMutableList()
            }
        }

        // update notification, only if it hasn't been shown before, or after change in status
        Logging.logVerbose(
            Logging.Category.CLIENT,
            "ClientNotification: notification needs update? "
                    + (mOldComputingStatus == -1) + " "
                    + activeTasksChanged + " "
                    + !notificationShown + " "
                    + (updatedStatus.computingStatus != mOldComputingStatus) + " "
                    + (updatedStatus.computingStatus == ClientStatus.COMPUTING_STATUS_SUSPENDED
                    && updatedStatus.computingSuspendReason != mOldSuspendReason)
        )
        if (mOldComputingStatus == -1 || activeTasksChanged
            || !notificationShown
            || updatedStatus.computingStatus != mOldComputingStatus || (updatedStatus.computingStatus == ClientStatus.COMPUTING_STATUS_SUSPENDED
                    && updatedStatus.computingSuspendReason != mOldSuspendReason)
        ) {

            // update, build and notify
            nm.notify(notificationId, buildNotification(updatedStatus, active, mOldActiveTasks))
            Logging.logVerbose(Logging.Category.CLIENT, "ClientNotification: update")
            notificationShown = true

            // save status for comparison next time
            mOldComputingStatus = updatedStatus.computingStatus
            mOldSuspendReason = updatedStatus.computingSuspendReason
        }

        // start foreground service, if requested
        // notification instance exists now, but might be out-dated (if screen is off)
        if (active && !foreground) {
            setForegroundState(service)
        }
    }

    // Notification must be built, before setting service to foreground!
    @VisibleForTesting
    internal fun setForegroundState(service: Monitor, foregroundState: Boolean = true) {
        if(foregroundState) {
            service.startForeground(notificationId, n);
            Logging.logInfo(Logging.Category.CLIENT, "ClientNotification.setForeground() start service as foreground.");
            foreground = true;
        }
        else {
            foreground = false;
            if (VERSION.SDK_INT >= VERSION_CODES.N) {
                service.stopForeground(STOP_FOREGROUND_REMOVE);
            }
            else {
                @Suppress("DEPRECATION")
                service.stopForeground(true);
            }
            notificationShown = false;
            Logging.logInfo(Logging.Category.CLIENT, "ClientNotification.setForeground() stop service as foreground.");
        }
    }

    @SuppressLint("InlinedApi")
    @VisibleForTesting
    internal fun buildNotification(
        status: ClientStatus,
        active: Boolean,
        activeTasks: List<Result>?
    ): Notification {
        // get current client computing status
        val computingStatus = status.computingStatus
        // get status strings from ClientStatus
        val statusDesc = status.currentStatusDescription
        val statusTitle = status.currentStatusTitle

        // build notification
        val nb = NotificationCompat.Builder(context, "main-channel")
        nb.setContentTitle(statusTitle)
            .setSmallIcon(getIcon(computingStatus, true))
            .setLargeIcon(context.getBitmapFromVectorDrawable(getIcon(computingStatus, false)))
            .setContentIntent(contentIntent)

        // adapt priority based on computing status
        // computing: IDLE and COMPUTING (see wakelock handling)
        if (active) {
            @Suppress("DEPRECATION")
            nb.priority = Notification.PRIORITY_HIGH
        } else {
            @Suppress("DEPRECATION")
            nb.priority = Notification.PRIORITY_LOW
        }

        // set action based on computing status
        if (computingStatus == ClientStatus.COMPUTING_STATUS_NEVER) {
            // add resume button
            // 0 - only text. Unify all versions of android with text button.
            nb.addAction(
                0,
                context.getString(R.string.menu_run_mode_enable), getActionIntent(2)
            )
        } else {
            // add suspend button
            // 0 - only text. Unify all versions of android with text button.
            nb.addAction(
                0,
                context.getString(R.string.menu_run_mode_disable), getActionIntent(1)
            )
        }

        // set tasks if computing
        if (computingStatus == ClientStatus.COMPUTING_STATUS_COMPUTING && activeTasks != null && activeTasks.isNotEmpty()) {
            // set summary text
            nb.setSubText(statusDesc)
            // set number as content info
            nb.setNumber(activeTasks.size)
            // set names in list
            val inboxStyle = NotificationCompat.InboxStyle()
            for ((_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, project, _, app) in activeTasks) {
                var line = if (project == null) "" else project.name
                line += ": "
                line += if (app == null) "" else app.displayName
                inboxStyle.addLine(line)
            }
            nb.setStyle(inboxStyle)
        } else {
            nb.setContentText(statusDesc)
        }
        n = nb.build()
        return n!!
    }

    // creates pending intent to service with specified action code
    private fun getActionIntent(actionCode: Int): PendingIntent {
        val si = Intent(context, Monitor::class.java)
        si.putExtra("action", actionCode)
        return PendingIntent.getService(context, 0, si, PendingIntent.FLAG_CANCEL_CURRENT)
    }

    // returns resource id of icon
    @VisibleForTesting
    internal fun getIcon(status: Int, isSmall: Boolean): Int {
        return when (status) {
            ClientStatus.COMPUTING_STATUS_NEVER, ClientStatus.COMPUTING_STATUS_SUSPENDED, ClientStatus.COMPUTING_STATUS_IDLE -> if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP && isSmall) {
                R.mipmap.ic_boinc_paused_white
            } else {
                R.drawable.ic_boinc_paused
            }
            else -> if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP && isSmall) {
                R.mipmap.ic_boinc_white
            } else {
                R.drawable.ic_boinc
            }
        }
    }

    init {
        val intent = Intent(context, BOINCActivity::class.java)
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        intent.putExtra("targetFragment", R.string.tab_tasks)
        val flags = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_CANCEL_CURRENT
        } else {
            Intent.FLAG_ACTIVITY_CLEAR_TOP or Intent.FLAG_ACTIVITY_NEW_TASK
        }
        contentIntent = PendingIntent.getActivity(context,0, intent, flags)
    }
}
