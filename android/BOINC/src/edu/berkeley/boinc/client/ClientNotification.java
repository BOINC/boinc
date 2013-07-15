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
	public synchronized void update(ClientStatus updatedStatus, Monitor monitorService) {
		
		// update notification, only after change in status
		if (clientNotification.mOldComputingStatus == -1 
				|| updatedStatus.computingStatus.intValue() != clientNotification.mOldComputingStatus
				|| (updatedStatus.computingStatus == ClientStatus.COMPUTING_STATUS_SUSPENDED
				&& updatedStatus.computingSuspendReason != clientNotification.mOldSuspendReason)) {
			
			if(updatedStatus.computingStatus == ClientStatus.COMPUTING_STATUS_COMPUTING) {
				// computing! set service as foreground
				monitorService.startForeground(notificationId, buildNotification(updatedStatus));
				if(Logging.DEBUG) Log.d(Logging.TAG,"ClientNotification.update() start service as foreground.");
				foreground = true;
			} else {
				// not computing, set service as background
				if(foreground) {
					foreground = false;
					monitorService.stopForeground(true);
					if(Logging.DEBUG) Log.d(Logging.TAG,"ClientNotification.update() stop service as foreground.");
				}
				// check whether notification is allowed in preferences	
				if (!Monitor.getAppPrefs().getShowNotification()) {
					enable(false);
					return;
				}

				nm.notify(notificationId, buildNotification(updatedStatus));
			}
			
			
			// save status for comparison next time
			clientNotification.mOldComputingStatus = updatedStatus.computingStatus;
			clientNotification.mOldSuspendReason = updatedStatus.computingSuspendReason;
		}
	}
	
	// called after change in notification preference
	public synchronized void enable(Boolean enable) {
		if(Logging.DEBUG) Log.d(Logging.TAG,"ClientNotification.enable() " + enable);
		if(foreground) {
			// foreground notification mandatory, do not change
			if(Logging.DEBUG) Log.d(Logging.TAG,"ClientNotification.enable() service in foreground, do not change.");
		} else {
			// service in background, notification behavior configurable
			if(enable){
				try{
					ClientStatus status = Monitor.getClientStatus();
					nm.notify(notificationId, buildNotification(status));
					// save status for comparison next time
					clientNotification.mOldComputingStatus = status.computingStatus;
					clientNotification.mOldSuspendReason = status.computingSuspendReason;
				} catch (Exception e) {if(Logging.WARNING) Log.w(Logging.TAG,"ClientNotification.enable() failed!");}
				
			} else {
				nm.cancel(notificationId);
				clientNotification.mOldComputingStatus = -1;
			}
		}
	}
	
	// cancels notification, called during client shutdown
	public synchronized void cancel() {
		nm.cancel(notificationId);
	}

	private Notification buildNotification(ClientStatus status) {
		// get current client computingstatus
		Integer computingStatus = status.computingStatus;
		// get status string from ClientStatus
		String statusText = status.getCurrentStatusString();
		
		// build notification
		Notification notification = new NotificationCompat.Builder(context)
        	.setContentTitle(context.getString(R.string.app_name))
        	.setContentText(statusText)
        	.setSmallIcon(getIcon(computingStatus))
        	.setLargeIcon(BitmapFactory.decodeResource(context.getResources(), getIcon(computingStatus)))
        	.setContentIntent(contentIntent)
        	.setOngoing(true)
        	.build();
		
		return notification;
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
