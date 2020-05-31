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
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.util.Log;

import androidx.core.app.NotificationCompat;
import androidx.core.content.ContextCompat;

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
    private ClientStatus clientStatus;
    private Context context;
    private PersistentStorage persistentStorage;

    private NotificationManager nm;
    private Integer notificationId;
    private PendingIntent contentIntent;

    private List<Notice> currentlyNotifiedNotices = new ArrayList<>();
    private boolean isNotificationShown = false;

    @Inject
    public NoticeNotification(Context context, ClientStatus clientStatus, PersistentStorage persistentStorage) {
        this.context = context;
        this.clientStatus = clientStatus;
        this.persistentStorage = persistentStorage;
        this.nm = ContextCompat.getSystemService(this.context, NotificationManager.class);
        notificationId = this.context.getResources().getInteger(R.integer.notice_notification_id);
        Intent intent = new Intent(this.context, BOINCActivity.class);
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
    public void update(List<Notice> notices, boolean isPreferenceEnabled) {
        if(!isPreferenceEnabled) {
            if(isNotificationShown) {
                nm.cancel(notificationId);
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
            nm.notify(notificationId, buildNotification());
            isNotificationShown = true;
        }
    }

    @SuppressLint("InlinedApi")
    private Notification buildNotification() {
        // build new notification from scratch every time a notice arrives
        final NotificationCompat.Builder nb;
        final int notices = currentlyNotifiedNotices.size();
        final String projectName = currentlyNotifiedNotices.get(0).getProjectName();

        nb = new NotificationCompat.Builder(context, "main-channel");
        nb.setContentTitle(context.getResources().getQuantityString(
                R.plurals.notice_notification, notices, projectName, notices)).
                  setSmallIcon(R.drawable.ic_baseline_email_white).
                  setAutoCancel(true).
                  setContentIntent(this.contentIntent);
        if(notices == 1) {
            // single notice view
            nb.setContentText(currentlyNotifiedNotices.get(0).getTitle()).
                    setLargeIcon(getLargeProjectIcon(context, projectName));
        }
        else {
            // multi notice view
            nb.setNumber(notices)
              .setLargeIcon(BOINCUtils.getBitmapFromVectorDrawable(context, R.drawable.ic_boinc))
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

    private Bitmap getLargeProjectIcon(final Context context, final String projectName) {
        final Bitmap projectIconBitmap;
        try {
            return (projectIconBitmap = clientStatus.getProjectIconByName(projectName)) != null ?
                   Bitmap.createScaledBitmap(
                           projectIconBitmap,
                           projectIconBitmap.getWidth() << 1,
                           projectIconBitmap.getHeight() << 1,
                           false
                   ) :
                   BOINCUtils.getBitmapFromVectorDrawable(context, R.drawable.ic_boinc);
        }
        catch(Exception e) {
            if(Log.isLoggable(Logging.TAG, Log.DEBUG)) {
                Log.d(
                        Logging.TAG,
                        e.getLocalizedMessage(),
                        e
                );
            }
            return BOINCUtils.getBitmapFromVectorDrawable(context, R.drawable.ic_boinc);
        }
    }
}
