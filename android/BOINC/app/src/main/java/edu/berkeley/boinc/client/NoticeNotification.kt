/*
  This file is part of BOINC.
  https://boinc.berkeley.edu
  Copyright (C) 2022 University of California

  BOINC is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License
  as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later version.

  BOINC is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
**/
package edu.berkeley.boinc.client

import android.annotation.SuppressLint
import android.app.Notification
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.net.Uri
import android.os.Build.VERSION
import android.os.Build.VERSION_CODES
import androidx.annotation.VisibleForTesting
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationCompat.BigTextStyle
import androidx.core.app.NotificationCompat.Builder
import androidx.core.app.NotificationCompat.InboxStyle
import androidx.core.app.NotificationManagerCompat
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.R.drawable
import edu.berkeley.boinc.R.integer
import edu.berkeley.boinc.R.mipmap
import edu.berkeley.boinc.R.plurals
import edu.berkeley.boinc.R.string
import edu.berkeley.boinc.rpc.Notice
import edu.berkeley.boinc.utils.Logging.Category.CLIENT
import edu.berkeley.boinc.utils.Logging.logException
import edu.berkeley.boinc.utils.getBitmapFromVectorDrawable
import javax.inject.Inject
import javax.inject.Singleton

@SuppressLint("UnspecifiedImmutableFlag")
@Singleton
class NoticeNotification @Inject constructor(
    private val context: Context,
    private val clientStatus: ClientStatus,
    private val persistentStorage: PersistentStorage
) {
    private val notificationManagerCompat: NotificationManagerCompat = NotificationManagerCompat.from(context)
    private val summaryNotificationID: Int = context.resources.getInteger(integer.notice_notification_id)
    private val contentIntent: PendingIntent
    private val notificationIDs: MutableList<Int> = ArrayList()
    @VisibleForTesting(otherwise = VisibleForTesting.PRIVATE)
    val currentlyNotifiedNotices: MutableList<Notice> = ArrayList()
    private var isNotificationShown = false

    init {
        val intent = Intent(context, BOINCActivity::class.java)
        intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP)
        intent.putExtra("targetFragment", string.tab_notices)
        contentIntent =
            PendingIntent.getActivity(context, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT)
    }

    /**
     * cancels currently shown notice and clears data
     * called when user clicks notice
     */
    fun cancelNotification() {
        if (isNotificationShown) {
            for (notificationID in notificationIDs) {
                notificationManagerCompat.cancel(notificationID)
            }
            notificationIDs.clear()
            notificationManagerCompat.cancel(summaryNotificationID)
            isNotificationShown = false
            currentlyNotifiedNotices.clear()
        }
    }

    /**
     * Updates notification with current notices
     */
    fun update(notices: List<Notice>, isPreferenceEnabled: Boolean) {
        if (!isPreferenceEnabled) {
            if (isNotificationShown) {
                notificationManagerCompat.cancel(summaryNotificationID)
                isNotificationShown = false
            }
            return
        }

        // filter new notices
        var newNotice = false
        var mostRecentSeenArrivalTime = 0.0
        val lastNotifiedArrivalTime = persistentStorage.lastNotifiedNoticeArrivalTime
        for (tmp in notices) {
            if (tmp.arrivalTime > lastNotifiedArrivalTime) {
                // multiple new notices might have same arrival time -> write back after adding all
                currentlyNotifiedNotices.add(tmp)
                newNotice = true
                if (tmp.arrivalTime > mostRecentSeenArrivalTime) {
                    mostRecentSeenArrivalTime = tmp.arrivalTime
                }
            }
        }
        if (newNotice) {
            // new notices came in
            persistentStorage.lastNotifiedNoticeArrivalTime = mostRecentSeenArrivalTime
            val notifications = buildNoticeNotifications()
            for (i in notifications.indices) {
                val notification = notifications[i]
                val noticeNotificationID = summaryNotificationID + i + 1
                notificationIDs.add(noticeNotificationID)
                notificationManagerCompat.notify(noticeNotificationID, notification)
            }
            notificationManagerCompat.notify(summaryNotificationID, buildSummaryNotification())
            isNotificationShown = true
        }
    }

    @VisibleForTesting(otherwise = VisibleForTesting.PRIVATE)
    fun buildNoticeNotifications(): List<Notification> {
        val notifications: MutableList<Notification> = ArrayList()
        for (notice in currentlyNotifiedNotices) {
            val intent = Intent(Intent.ACTION_VIEW, Uri.parse(notice.link))
            val browserIntent = PendingIntent.getActivity(context, 0, intent, 0)

            val builder = Builder(context, "notice-channel")
                .setAutoCancel(true)
                .setContentIntent(browserIntent)
                .setContentTitle("${notice.projectName}: ${notice.title}")
                .setContentText(notice.description)
                .setStyle(BigTextStyle().bigText(notice.description))
                .setLargeIcon(getLargeProjectIcon(context ,notice.projectName))
                .setSmallIcon(drawable.ic_boinc_notice)
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                .setGroup(NOTICE_GROUP)

            notifications.add(builder.build())
        }

        return notifications
    }

    @VisibleForTesting(otherwise = VisibleForTesting.PRIVATE)
    fun buildSummaryNotification(): Notification {
        val notices = currentlyNotifiedNotices.size
        val projectName = currentlyNotifiedNotices[0].projectName
        val icon = drawable.ic_boinc
        val smallIcon =
            if (VERSION.SDK_INT < VERSION_CODES.LOLLIPOP) mipmap.ic_boinc_notice_white else drawable.ic_boinc_notice
        // build new notification from scratch every time a notice arrives
        val nb = Builder(context, "notice-channel")
            .setContentTitle(
                context.resources.getQuantityString(
                    plurals.notice_notification,
                    notices, projectName, notices
                )
            )
            .setSmallIcon(smallIcon)
            .setAutoCancel(true)
            .setContentIntent(contentIntent)
        if (notices == 1) {
            // single notice view
            nb.setContentText(currentlyNotifiedNotices[0].title)
                .setLargeIcon(getLargeProjectIcon(context, projectName))
        } else {
            // multi notice view
            nb.setNumber(notices)
                .setLargeIcon(context.getBitmapFromVectorDrawable(icon))
            if (VERSION.SDK_INT < VERSION_CODES.N) {
                nb.setSubText(context.getString(string.app_name))
            }

            // append notice titles to list
            val inboxStyle = InboxStyle()
            for (notice in currentlyNotifiedNotices) {
                inboxStyle.addLine("${notice.projectName}: ${notice.title}")
            }
            nb.setStyle(inboxStyle)
        }
        nb.setGroup(NOTICE_GROUP)
            .setGroupSummary(true)
        return nb.build()
    }

    @VisibleForTesting(otherwise = VisibleForTesting.PRIVATE)
    fun getLargeProjectIcon(context: Context, projectName: String): Bitmap {
        val projectIconBitmap = clientStatus.getProjectIconByName(projectName)
        val icon = drawable.ic_boinc
        return try {
            if (projectIconBitmap != null) Bitmap.createScaledBitmap(
                projectIconBitmap,
                projectIconBitmap.width shl 1,
                projectIconBitmap.height shl 1,
                false
            ) else context.getBitmapFromVectorDrawable(icon)
        } catch (e: Exception) {
            logException(CLIENT, e.localizedMessage as String, e)
            context.getBitmapFromVectorDrawable(icon)
        }
    }

    companion object {
        private const val NOTICE_GROUP = "edu.berkeley.boinc.NOTICES"
    }
}
