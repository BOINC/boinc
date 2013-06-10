package edu.berkeley.boinc.client;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.support.v4.app.NotificationCompat;

public class ClientNotification {
	//private static final String TAG = "ClientNotification";

	private static ClientNotification clientNotification = null;
	
	private Context context;
	private NotificationManager nm;
	private Integer notificationId;
	private PendingIntent contentIntent;

	private int mOldComputingStatus = -1;
	private int mOldSuspendReason = -1;

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
		// check whether notification is allowed in preferences	
		if (!Monitor.getAppPrefs().getShowNotification()) {
			nm.cancel(notificationId);
			clientNotification.mOldComputingStatus = -1;
			return;
		}
		
		// get current client status
		ClientStatus updatedStatus = Monitor.getClientStatus();
		
		// update notification
		if (clientNotification.mOldComputingStatus == -1 
				|| updatedStatus.computingStatus.intValue() != clientNotification.mOldComputingStatus
				|| (updatedStatus.computingStatus == ClientStatus.COMPUTING_STATUS_SUSPENDED
				&& updatedStatus.computingSuspendReason != clientNotification.mOldSuspendReason)) {
			
			nm.notify(notificationId, buildNotification());
			
			// save status for comparison next time
			clientNotification.mOldComputingStatus = updatedStatus.computingStatus;
			clientNotification.mOldSuspendReason = updatedStatus.computingSuspendReason;
		}
	}
	
	// cancels notification, called during client shutdown
	public synchronized void cancel() {
		nm.cancel(notificationId);
	}

	private Notification buildNotification() {
		// get current client computingstatus
		Integer computingStatus = Monitor.getClientStatus().computingStatus;
		// get status string from ClientStatus
		String statusText = Monitor.getClientStatus().getCurrentStatusString();
		
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
