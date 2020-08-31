/*
  This file is part of BOINC.
  http://boinc.berkeley.edu
  Copyright (C) 2016 University of California
  
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
package edu.berkeley.boinc.client;

import android.app.Notification;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Build;
import android.util.Log;

import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import java.util.ArrayList;
import java.util.List;

import javax.inject.Inject;
import javax.inject.Singleton;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.Notice;
import edu.berkeley.boinc.utils.BOINCUtils;
import edu.berkeley.boinc.utils.Logging;

@Singleton
public class NoticeNotification {
    private static final String NOTICE_GROUP = "edu.berkeley.boinc.NOTICES";

    private final ClientStatus clientStatus;
    private final Context context;
    private final PersistentStorage persistentStorage;

    private final NotificationManagerCompat notificationManagerCompat;
    private final int summaryNotificationID;
    private final PendingIntent contentIntent;

    private final List<Integer> notificationIDs = new ArrayList<>();
    private final List<Notice> currentlyNotifiedNotices = new ArrayList<>();
    private boolean isNotificationShown = false;

    @Inject
    public NoticeNotification(Context context, ClientStatus clientStatus, PersistentStorage persistentStorage) {
        this.context = context;
        this.clientStatus = clientStatus;
        this.persistentStorage = persistentStorage;
        this.notificationManagerCompat = NotificationManagerCompat.from(context);
        summaryNotificationID = context.getResources().getInteger(R.integer.notice_notification_id);
        Intent intent = new Intent(context, BOINCActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
        intent.putExtra("targetFragment", R.string.tab_notices);
        contentIntent = PendingIntent.getActivity(context, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);
    }

    /**
     * cancels currently shown notice and clears data
     * called when user clicks notice
     */
    public void cancelNotification() {
        if(isNotificationShown) {
            for (int notificationID : notificationIDs) {
                notificationManagerCompat.cancel(notificationID);
            }
            notificationIDs.clear();
            notificationManagerCompat.cancel(summaryNotificationID);
            isNotificationShown = false;
            currentlyNotifiedNotices.clear();
        }
    }

    /**
     * Updates notification with current notices
     */
    public void update(List<Notice> notices, boolean isPreferenceEnabled) {
        if(!isPreferenceEnabled) {
            if(isNotificationShown) {
                notificationManagerCompat.cancel(summaryNotificationID);
                isNotificationShown = false;
            }
            return;
        }

        // filter new notices
        boolean newNotice = false;
        double mostRecentSeenArrivalTime = 0;
        double lastNotifiedArrivalTime = persistentStorage.getLastNotifiedNoticeArrivalTime();

        for(Notice tmp : notices) {
            if(tmp.getArrivalTime() > lastNotifiedArrivalTime) {
                // multiple new notices might have same arrival time -> write back after adding all
                currentlyNotifiedNotices.add(tmp);
                newNotice = true;
                if(tmp.getArrivalTime() > mostRecentSeenArrivalTime) {
                    mostRecentSeenArrivalTime = tmp.getArrivalTime();
                }
            }
        }

        if(newNotice) {
            // new notices came in
            persistentStorage.setLastNotifiedNoticeArrivalTime(mostRecentSeenArrivalTime);
            final List<Notification> notifications = buildNoticeNotifications();
            for (int i = 0; i < notifications.size(); i++) {
                final Notification notification = notifications.get(i);
                final int noticeNotificationID = summaryNotificationID + i + 1;
                notificationIDs.add(noticeNotificationID);
                notificationManagerCompat.notify(noticeNotificationID, notification);
            }
            notificationManagerCompat.notify(summaryNotificationID, buildSummaryNotification());
            isNotificationShown = true;
        }
    }

    private List<Notification> buildNoticeNotifications() {
        final List<Notification> notifications = new ArrayList<>();

        for (Notice notice : currentlyNotifiedNotices) {
            final Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(notice.getLink()));
            final PendingIntent browserIntent = PendingIntent.getActivity(context, 0, intent, 0);

            final NotificationCompat.Builder builder = new NotificationCompat.Builder(context, "main-channel")
                    .setAutoCancel(true)
                    .setContentIntent(browserIntent)
                    .setContentTitle(notice.getProjectName() + ": " + notice.getTitle())
                    .setContentText(notice.getDescription())
                    .setStyle(new NotificationCompat.BigTextStyle().bigText(notice.getDescription()))
                    .setLargeIcon(getLargeProjectIcon(context, notice.getProjectName()))
                    .setSmallIcon(R.drawable.ic_boinc_notice)
                    .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                    .setGroup(NOTICE_GROUP);

            notifications.add(builder.build());
        }

        return notifications;
    }

    private Notification buildSummaryNotification() {
        final int notices = currentlyNotifiedNotices.size();
        final String projectName = currentlyNotifiedNotices.get(0).getProjectName();
        final int icon = Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP ? R.mipmap.ic_launcher : R.drawable.ic_boinc;
        final int smallIcon = Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP ?
                              R.mipmap.ic_boinc_notice_white : R.drawable.ic_boinc_notice;
        // build new notification from scratch every time a notice arrives
        final NotificationCompat.Builder nb = new NotificationCompat.Builder(context, "main-channel")
                .setContentTitle(context.getResources().getQuantityString(R.plurals.notice_notification,
                                                                          notices, projectName, notices))
                .setSmallIcon(smallIcon)
                .setAutoCancel(true)
                .setContentIntent(contentIntent);
        if(notices == 1) {
            // single notice view
            nb.setContentText(currentlyNotifiedNotices.get(0).getTitle())
              .setLargeIcon(getLargeProjectIcon(context, projectName));
        }
        else {
            // multi notice view
            nb.setNumber(notices)
              .setLargeIcon(BOINCUtils.getBitmapFromVectorDrawable(context, icon));
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
                nb.setSubText(context.getString(R.string.app_name));
            }

            // append notice titles to list
            final NotificationCompat.InboxStyle inboxStyle = new NotificationCompat.InboxStyle();
            for (Notice notice : currentlyNotifiedNotices) {
                inboxStyle.addLine(notice.getProjectName() + ": " + notice.getTitle());
            }
            nb.setStyle(inboxStyle);
        }
        nb.setGroup(NOTICE_GROUP)
          .setGroupSummary(true);
        return nb.build();
    }

    private Bitmap getLargeProjectIcon(final Context context, final String projectName) {
        final Bitmap projectIconBitmap = clientStatus.getProjectIconByName(projectName);
        final int icon = Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP ? R.mipmap.ic_launcher : R.drawable.ic_boinc;
        try {
            return projectIconBitmap != null ?
                   Bitmap.createScaledBitmap(
                           projectIconBitmap,
                           projectIconBitmap.getWidth() << 1,
                           projectIconBitmap.getHeight() << 1,
                           false
                   ) :
                   BOINCUtils.getBitmapFromVectorDrawable(context, icon);
        }
        catch(Exception e) {
            if(Log.isLoggable(Logging.TAG, Log.DEBUG)) {
                Log.d(
                        Logging.TAG,
                        e.getLocalizedMessage(),
                        e
                );
            }
            return BOINCUtils.getBitmapFromVectorDrawable(context, icon);
        }
    }
}
