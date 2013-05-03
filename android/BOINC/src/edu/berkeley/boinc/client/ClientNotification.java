package edu.berkeley.boinc.client;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;

public class ClientNotification {
	private static final String TAG = "ClientNotification";

	private static final int NOTIFICATION_ID = 460;

	private static ClientNotification clientNotification = null;

	private boolean mIsEnabled = true;
	private int mOldComputingStatus = -1;

	private ClientNotification() {

	}

	/**
	 * Returns a reference to a singleton ClientNotification object.
	 * Constructs a new instance of the ClientNotification if not already constructed.
	 * @return ClientNotification static instance
	 */
	public static synchronized ClientNotification getInstance() {
		if (clientNotification == null)
			clientNotification = new ClientNotification();

		return clientNotification;
	}

	/**
	 * Updates notification with client's current status
	 * @param context
	 * @param updatedStatus new status
	 */
	public synchronized void update(Context context, ClientStatus updatedStatus) {
		if (clientNotification.mOldComputingStatus == -1 
				|| updatedStatus.computingStatus.intValue() != clientNotification.mOldComputingStatus) {
			if (clientNotification.mIsEnabled)
				updateNotification(context, updatedStatus.computingStatus);
			clientNotification.mOldComputingStatus = updatedStatus.computingStatus;
		}
	}

	/**
	 * Set notification enabled/disabled
	 * @param context
	 * @param enabled
	 */
	public synchronized void enable(Context context, boolean enabled) {
		clientNotification.mIsEnabled = enabled;
		if (clientNotification.mIsEnabled) {
			if (clientNotification.mOldComputingStatus != -1)
				updateNotification(context, clientNotification.mOldComputingStatus);
		} else {
			hide(context);
		}
	}

	private void updateNotification(Context context, int status) {
		switch(status) {
		case ClientStatus.COMPUTING_STATUS_NEVER:
//			hide(context);
//			break;
		case ClientStatus.COMPUTING_STATUS_SUSPENDED:
		case ClientStatus.COMPUTING_STATUS_IDLE:
			show(context, R.drawable.ic_stat_notify_boinc_paused, R.string.status_idle, BOINCActivity.class);
			break;
		case ClientStatus.COMPUTING_STATUS_COMPUTING:
			show(context, R.drawable.ic_stat_notify_boinc_normal, R.string.status_running, BOINCActivity.class);
			break;
		}
	}

	private void show(Context context, int icon, int message, Class<?> launchActivity) {

		// Set the icon, scrolling text and time-stamp
		Notification notification = new Notification(
				icon, 
				context.getText(message),
				System.currentTimeMillis());
		// The PendingIntent to launch activity
		PendingIntent pendingIntent = PendingIntent.getActivity(context, 0,
				new Intent(context, launchActivity), 0);

		// Set the info for the views that show in the notification panel.
		notification.setLatestEventInfo(context, context.getText(R.string.app_name),
				context.getText(message), pendingIntent);
		notification.flags |= Notification.FLAG_NO_CLEAR;

		NotificationManager nm = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
		nm.notify(NOTIFICATION_ID, notification);
	}

	private void hide(Context context) {
		NotificationManager nm = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
		nm.cancel(NOTIFICATION_ID);
	}
}
