package edu.berkeley.boinc.client;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.Logging;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

public class ClientNotification {
	//private static final String TAG = "ClientNotification";

	private static ClientNotification clientNotification = null;
	
	private Context context;
	private NotificationManager nm;
	private Integer notificationId;
	private PendingIntent contentIntent;
	private Notification n = null;
	
	private int mOldComputingStatus = -1;
	private int mOldSuspendReason = -1;
	// debug foreground state by running
	// adb shell: dumpsys activity services edu.berkeley.boinc
	private boolean foreground = false;

	/**
	 * Returns a reference to a singleton ClientNotification object.
	 * Constructs a new instance of the ClientNotification if not already constructed.
	 * @return ClientNotification static instance
	 */
	public static synchronized ClientNotification getInstance(Context ctx) {
		if (clientNotification == null) {
			clientNotification = new ClientNotification(ctx);
		}
		return clientNotification;
	}
	
	public ClientNotification (Context ctx) {
		this.context = ctx;
		this.nm = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
		notificationId = context.getResources().getInteger(R.integer.autostart_notification_id);
		Intent intent = new Intent(context, BOINCActivity.class);
		intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
		contentIntent = PendingIntent.getActivity(context, 0, intent, 0);
	}

	/**
	 * Updates notification with client's current status
	 */
	public synchronized void update() {
		try {
			ClientStatus updatedStatus = Monitor.getClientStatus();
			
			// update notification, only after change in status
			if (clientNotification.mOldComputingStatus == -1 
					|| updatedStatus.computingStatus.intValue() != clientNotification.mOldComputingStatus
					|| (updatedStatus.computingStatus == ClientStatus.COMPUTING_STATUS_SUSPENDED
					&& updatedStatus.computingSuspendReason != clientNotification.mOldSuspendReason)) {
				
				buildNotification(updatedStatus);
				
				// update notification
				if(foreground || Monitor.getAppPrefs().getShowNotification()) nm.notify(notificationId, n);
				
				// save status for comparison next time
				clientNotification.mOldComputingStatus = updatedStatus.computingStatus;
				clientNotification.mOldSuspendReason = updatedStatus.computingSuspendReason;
			}
		} catch (Exception e) {if(Logging.WARNING) Log.d(Logging.TAG, "ClientNotification.update failed.");}
	}
	
	// called by Monitor to enable foreground with notification
	public synchronized void setForeground(Boolean setForeground, Monitor service) {
		if(foreground != setForeground) {
			if(setForeground) {
				// check whether notification is available
				if (n == null) update();
				// set service foreground
				service.startForeground(notificationId, n);
				if(Logging.DEBUG) Log.d(Logging.TAG,"ClientNotification.setForeground() start service as foreground.");
				foreground = true;
			} else {
				// set service background
				foreground = false;
				Boolean remove = !Monitor.getAppPrefs().getShowNotification();
				service.stopForeground(remove);
				if(Logging.DEBUG) Log.d(Logging.TAG,"ClientNotification.setForeground() stop service as foreground.");
			}
		}
	}
	
	// cancels notification, called during client shutdown or when disabling preference
	public synchronized void cancel() {
		nm.cancel(notificationId);
		clientNotification.mOldComputingStatus = -1;
	}

	private Notification buildNotification(ClientStatus status) {
		// get current client computingstatus
		Integer computingStatus = status.computingStatus;
		// get status string from ClientStatus
		String statusText = status.getCurrentStatusString();
		
		// build notification
		n = new NotificationCompat.Builder(context)
        	.setContentTitle(context.getString(R.string.app_name))
        	.setContentText(statusText)
        	.setSmallIcon(getIcon(computingStatus))
        	.setLargeIcon(BitmapFactory.decodeResource(context.getResources(), getIcon(computingStatus)))
        	.setContentIntent(contentIntent)
        	.setOngoing(true)
        	.build();
		
		return n;
	}

	// returns resource id of icon
	private int getIcon(int status) {
		int icon;
		switch(status) {
		case ClientStatus.COMPUTING_STATUS_NEVER:
			icon = R.drawable.ic_stat_notify_boinc_paused;
			break;
		case ClientStatus.COMPUTING_STATUS_SUSPENDED:
			icon = R.drawable.ic_stat_notify_boinc_paused;
			break;
		case ClientStatus.COMPUTING_STATUS_IDLE:
			icon = R.drawable.ic_stat_notify_boinc_paused;
			break;
		case ClientStatus.COMPUTING_STATUS_COMPUTING:
			icon = R.drawable.ic_stat_notify_boinc_normal;
			break;
		default:
			icon = R.drawable.ic_stat_notify_boinc_normal;
		}
		return icon;
	}
}
