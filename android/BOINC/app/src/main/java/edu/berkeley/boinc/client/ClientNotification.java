package edu.berkeley.boinc.client;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.util.Log;

import androidx.core.app.NotificationCompat;
import androidx.core.content.ContextCompat;
import androidx.core.graphics.drawable.DrawableCompat;
import androidx.vectordrawable.graphics.drawable.VectorDrawableCompat;

import java.util.ArrayList;
import java.util.List;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.Result;
import edu.berkeley.boinc.utils.Logging;

public class ClientNotification {

    private static ClientNotification clientNotification = null;

    private Context context;
    private NotificationManager nm;
    private Integer notificationId;
    private PendingIntent contentIntent;
    private Notification n;

    private int mOldComputingStatus = -1;
    private int mOldSuspendReason = -1;
    private List<Result> mOldActiveTasks = new ArrayList<>();
    private boolean notificationShown = false;
    // debug foreground state by running
    // adb shell: dumpsys activity services edu.berkeley.boinc
    private boolean foreground = false;

    /**
     * Returns a reference to a singleton ClientNotification object.
     * Constructs a new instance of the ClientNotification if not already constructed.
     *
     * @return ClientNotification static instance
     */
    public static ClientNotification getInstance(Context ctx) {
        if(clientNotification == null) {
            clientNotification = new ClientNotification(ctx);
        }
        return clientNotification;
    }

    private ClientNotification(Context ctx) {
        this.context = ctx;
        this.nm = ContextCompat.getSystemService(context, NotificationManager.class);
        notificationId = context.getResources().getInteger(R.integer.autostart_notification_id);
        Intent intent = new Intent(context, BOINCActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        contentIntent = PendingIntent.getActivity(context, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);
    }

    /**
     * Updates notification with client's current status. Notifies if not present. Checking
     * notification-related preferences.
     *
     * @param updatedStatus client status data
     * @param service       reference to service, sets to foreground if active
     * @param active        indicator whether BOINC should stay in foreground (during computing and
     *                      idle, i.e. not suspended)
     */
    public void update(ClientStatus updatedStatus, Monitor service, Boolean active) {
        // nop if data is not present
        if(service == null || updatedStatus == null) {
            return;
        }

        //check if active tasks have changed to force update
        boolean activeTasksChanged = false;
        if(active.equals(Boolean.TRUE) &&
           updatedStatus.computingStatus == ClientStatus.COMPUTING_STATUS_COMPUTING) {
            List<Result> activeTasks = updatedStatus.getExecutingTasks();
            if(activeTasks.size() != mOldActiveTasks.size()) {
                activeTasksChanged = true;
            }
            else {
                for(int x = 0; x < activeTasks.size(); x++) {
                    if(!activeTasks.get(x).getName().equals(mOldActiveTasks.get(x).getName())) {
                        activeTasksChanged = true;
                        Log.d(Logging.TAG, "Active task: " + activeTasks.get(x).getName()
                                           + ", old active task: " + mOldActiveTasks.get(x).getName());
                        break;
                    }
                }
            }
            if(activeTasksChanged) {
                mOldActiveTasks = activeTasks;
            }
        }
        else if (!mOldActiveTasks.isEmpty()) {
            mOldActiveTasks.clear();
            activeTasksChanged = true;
        }

        // update notification, only if it hasn't been shown before, or after change in status
        if(Logging.VERBOSE) {
            Log.d(Logging.TAG,
                  "ClientNotification: notification needs update? "
                  + (clientNotification.mOldComputingStatus == -1)
                  + activeTasksChanged
                  + !notificationShown
                  + (updatedStatus.computingStatus != clientNotification.mOldComputingStatus)
                  + (updatedStatus.computingStatus == ClientStatus.COMPUTING_STATUS_SUSPENDED
                     && updatedStatus.computingSuspendReason != clientNotification.mOldSuspendReason));
        }
        if(clientNotification.mOldComputingStatus == -1
           || activeTasksChanged
           || !notificationShown
           || updatedStatus.computingStatus != clientNotification.mOldComputingStatus
           || (updatedStatus.computingStatus == ClientStatus.COMPUTING_STATUS_SUSPENDED
               && updatedStatus.computingSuspendReason != clientNotification.mOldSuspendReason)) {

            // update, build and notify
            nm.notify(notificationId, buildNotification(updatedStatus, active, mOldActiveTasks));
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "ClientNotification: update");
            }
            notificationShown = true;

            // save status for comparison next time
            clientNotification.mOldComputingStatus = updatedStatus.computingStatus;
            clientNotification.mOldSuspendReason = updatedStatus.computingSuspendReason;
        }

        // start foreground service, if requested
        // notification instance exists now, but might be out-dated (if screen is off)
        if(active.equals(Boolean.TRUE) && !foreground) {
            setForegroundState(service, true);
        }
    }

    // Notification must be built, before setting service to foreground!
    private void setForegroundState(Monitor service, Boolean foregroundState) {
        if(foregroundState) {
            service.startForeground(notificationId, n);
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "ClientNotification.setForeground() start service as foreground.");
            }
            foreground = true;
        }
        else {
            foreground = false;
            service.stopForeground(true);
            notificationShown = false;
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "ClientNotification.setForeground() stop service as foreground.");
            }
        }
    }

    @SuppressLint("InlinedApi")
    private Notification buildNotification(ClientStatus status, Boolean active, List<Result> activeTasks) {
        // get current client computingstatus
        Integer computingStatus = status.computingStatus;
        // get status strings from ClientStatus
        String statusDesc = status.getCurrentStatusDescription();
        String statusTitle = status.getCurrentStatusTitle();

        // build notification
        NotificationCompat.Builder nb = new NotificationCompat.Builder(context, "main-channel");
        nb.setContentTitle(statusTitle)
          .setSmallIcon(getIcon(computingStatus))
          .setLargeIcon(BOINCActivity.getBitmapFromVectorDrawable(context, getIcon(computingStatus)))
          .setContentIntent(contentIntent);

        // adapt priority based on computing status
        // computing: IDLE and COMPUTING (see wakelock handling)
        if(active) {
            nb.setPriority(Notification.PRIORITY_HIGH);
        }
        else {
            nb.setPriority(Notification.PRIORITY_LOW);
        }

        // set action based on computing status
        if(computingStatus == ClientStatus.COMPUTING_STATUS_NEVER) {
            // add resume button
            // 0 - only text. Unify all versions of android with text button.
            nb.addAction(0,
                         context.getString(R.string.menu_run_mode_enable), getActionIntent(2));
        }
        else {
            // add suspend button
            // 0 - only text. Unify all versions of android with text button.
            nb.addAction(0,
                         context.getString(R.string.menu_run_mode_disable), getActionIntent(1));
        }

        // set tasks if computing
        if(computingStatus == ClientStatus.COMPUTING_STATUS_COMPUTING) {
            // set summary text
            nb.setSubText(statusDesc);
            // set number as content info
            nb.setNumber(activeTasks.size());
            // set names in list
            NotificationCompat.InboxStyle inboxStyle = new NotificationCompat.InboxStyle();
            for(Result task : activeTasks) {
                inboxStyle.addLine(task.getProject().getName() + ": " + task.getApp().getDisplayName());
            }
            nb.setStyle(inboxStyle);
        }
        else {
            nb.setContentText(statusDesc);
        }

        n = nb.build();

        return n;
    }

    // creates pending intent to service with specified action code
    private PendingIntent getActionIntent(int actionCode) {
        Intent si = new Intent(context, Monitor.class);
        si.putExtra("action", actionCode);
        return PendingIntent.getService(context, 0, si, PendingIntent.FLAG_CANCEL_CURRENT);
    }

    // returns resource id of icon
    private int getIcon(int status) {
        int icon;
        switch(status) {
            case ClientStatus.COMPUTING_STATUS_NEVER:
            case ClientStatus.COMPUTING_STATUS_SUSPENDED:
            case ClientStatus.COMPUTING_STATUS_IDLE:
                icon = R.drawable.ic_stat_notify_boinc_paused;
                break;
            default:
                icon = R.drawable.ic_boinc;
        }
        return icon;
    }
}
