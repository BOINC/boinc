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

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import androidx.annotation.RequiresApi;
import androidx.core.app.NotificationCompat;

import android.graphics.Color;
import android.os.Build;
import android.util.Log;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.Notice;
import edu.berkeley.boinc.utils.Logging;

import java.util.ArrayList;
import java.util.List;

public class NoticeNotification {
    private static NoticeNotification noticeNotification = null;

    private Context context;
    private PersistentStorage store;
    private NotificationManager nm;
    private Integer notificationId;
    private PendingIntent contentIntent;

    private ArrayList<Notice> currentlyNotifiedNotices = new ArrayList<>();
    private Boolean isNotificationShown = false;

    /**
     * Returns a reference to a singleton noticeNotification object.
     * Constructs a new instance of the noticeNotification if not already constructed.
     *
     * @return noticeNotification static instance
     */
    public static NoticeNotification getInstance(Context ctx) {
        if(noticeNotification == null) {
            noticeNotification = new NoticeNotification(ctx);
        }
        return noticeNotification;
    }

    public NoticeNotification(Context ctx) {
        this.context = ctx;
        this.store = new PersistentStorage(ctx);
        this.nm = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationId = context.getResources().getInteger(R.integer.notice_notification_id);
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
            nm.cancel(notificationId);
            isNotificationShown = false;
            currentlyNotifiedNotices.clear();
        }
    }

    /**
     * Updates notification with current notices
     */
    public void update(List<Notice> notices, Boolean isPreferenceEnabled) {
        if(!isPreferenceEnabled) {
            if(isNotificationShown) {
                nm.cancel(notificationId);
                isNotificationShown = false;
            }
            return;
        }

        // filter new notices
        Boolean newNotice = false;
        double mostRecentSeenArrivalTime = 0;
        double lastNotifiedArrivalTime = store.getLastNotifiedNoticeArrivalTime();

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
            store.setLastNotifiedNoticeArrivalTime(mostRecentSeenArrivalTime);
            nm.notify(notificationId, buildNotification());
            isNotificationShown = true;
        }
    }

    @RequiresApi(Build.VERSION_CODES.O)
    private String createNotificationChannel(String channelId, String channelName) {
        NotificationChannel chan = new NotificationChannel(channelId,
                                                           channelName, NotificationManager.IMPORTANCE_NONE);
        chan.setLightColor(Color.BLUE);
        chan.setDescription(context.getString(R.string.main_notification_channel_description));
        chan.setLockscreenVisibility(Notification.VISIBILITY_PRIVATE);
        NotificationManager service = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        service.createNotificationChannel(chan);
        return channelId;
    }

    @SuppressLint("InlinedApi")
    private Notification buildNotification() {
        // build new notification from scratch every time a notice arrives
        final int notices = currentlyNotifiedNotices.size();
        final String projectName = currentlyNotifiedNotices.get(0).getProjectName();

        String channelId;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            final String channelName = context.getString(R.string.main_notification_channel_name);
            channelId = createNotificationChannel("main-channel", channelName);
        } else {
            // If earlier version channel ID is not used
            // https://developer.android.com/reference/android/support/v4/app/NotificationCompat.Builder.html#NotificationCompat.Builder(android.content.Context)
            channelId = "main-channel";
        }

        final NotificationCompat.Builder nb = new NotificationCompat.Builder(context, channelId);
        nb.setContentTitle(context.getResources().getQuantityString(
                R.plurals.notice_notification, notices, projectName, notices))
          .setSmallIcon(R.drawable.mailw)
          .setAutoCancel(true)
          .setContentIntent(this.contentIntent)
        if(notices == 1) {
            // single notice view
            nb.setContentText(currentlyNotifiedNotices.get(0).getTitle()).
                    setLargeIcon(NoticeNotification.getLargeProjectIcon(context, projectName));
        }
        else {
            // multi notice view
            nb.setNumber(notices)
              .setLargeIcon(BitmapFactory.decodeResource(
                      this.context.getResources(),
                      R.drawable.ic_stat_notify_boinc_normal))
              .setSubText(this.context.getString(R.string.app_name));

            // append notice titles to list
            final NotificationCompat.InboxStyle inboxStyle = new NotificationCompat.InboxStyle();
            for(int i = 0; i < notices; i++) {
                final Notice notice;
                inboxStyle.addLine((notice = this.currentlyNotifiedNotices.get(i)).getProjectName() +
                                   ": " + notice.getTitle());
            }
            nb.setStyle(inboxStyle);
        }
        return nb.build();
    }

    private static final Bitmap getLargeProjectIcon(final Context context, final String projectName) {
        final Bitmap projectIconBitmap;
        try {
            return (projectIconBitmap = Monitor.getClientStatus().getProjectIconByName(projectName)) != null ?
                   Bitmap.createScaledBitmap(
                           projectIconBitmap,
                           projectIconBitmap.getWidth() << 1,
                           projectIconBitmap.getHeight() << 1,
                           false
                   ) :
                   BitmapFactory.decodeResource(
                           context.getResources(),
                           R.drawable.ic_stat_notify_boinc_normal
                   );
        }
        catch(Exception e) {
            if(Log.isLoggable(Logging.TAG, Log.DEBUG)) {
                Log.d(
                        Logging.TAG,
                        e.getLocalizedMessage(),
                        e
                );
            }
            return BitmapFactory.decodeResource(
                    context.getResources(),
                    R.drawable.ic_stat_notify_boinc_normal
            );
        }
    }
}
