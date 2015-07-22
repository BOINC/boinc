package edu.berkeley.boinc.client;

import java.util.ArrayList;
import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.Notice;
import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v4.app.NotificationCompat;

public class NoticeNotification {

	private static NoticeNotification noticeNotification = null;
	
	private Context context;
	private PersistentStorage store;
	private NotificationManager nm;
	private Integer notificationId;
	private PendingIntent contentIntent;
	private Notification n;
	
	private ArrayList<Notice> currentlyNotifiedNotices = new ArrayList<Notice>();
	private Boolean isNotificationShown = false;

	//private Boolean debug = true;
	/**
	 * Returns a reference to a singleton noticeNotification object.
	 * Constructs a new instance of the noticeNotification if not already constructed.
	 * @return noticeNotification static instance
	 */
	public static NoticeNotification getInstance(Context ctx) {
		if (noticeNotification == null) {
			noticeNotification = new NoticeNotification(ctx);
		}
		return noticeNotification;
	}
	
	public NoticeNotification (Context ctx) {
		this.context = ctx;
		this.store = new PersistentStorage(ctx);
		this.nm = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
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
	public void update(ArrayList<Notice> notices, Boolean isPreferenceEnabled) {
		
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
		/*
		if(debug) {
			lastNotifiedArrivalTime = 0;
			debug = false;
		}*/
		for(Notice tmp : notices) {
			if(tmp.arrival_time > lastNotifiedArrivalTime) {
				// multiple new notices might have same arrival time -> write back after adding all
				currentlyNotifiedNotices.add(tmp);
				newNotice = true;
				if(tmp.arrival_time > mostRecentSeenArrivalTime) mostRecentSeenArrivalTime = tmp.arrival_time;
			}
		}
		
		if(newNotice) {
			// new notices came in
			store.setLastNotifiedNoticeArrivalTime(mostRecentSeenArrivalTime);
			nm.notify(notificationId, buildNotification());
			isNotificationShown = true;
		}
	}

	@SuppressLint("InlinedApi")
	private Notification buildNotification() {
		// build new notification from scratch every time a notice arrives
		NotificationCompat.Builder nb = new NotificationCompat.Builder(context);
		
		if(currentlyNotifiedNotices.size() == 1) {
			// single notice view
			nb.setContentTitle(context.getString(R.string.notice_notification_single_header) + " " + currentlyNotifiedNotices.get(0).project_name)
				.setContentText(currentlyNotifiedNotices.get(0).title)
	        	.setSmallIcon(R.drawable.mailw)
	        	.setContentIntent(contentIntent);
			
			// scale project image
			try {
				Bitmap unscaled = Monitor.getClientStatus().getProjectIconByName(currentlyNotifiedNotices.get(0).project_name);
				if(unscaled != null) {
					Bitmap scaled = Bitmap.createScaledBitmap(unscaled, unscaled.getWidth() * 2, unscaled.getHeight() * 2, false);
					nb.setLargeIcon(scaled);
				} else {
					nb.setLargeIcon(BitmapFactory.decodeResource(context.getResources(), R.drawable.ic_stat_notify_boinc_normal));
				}
			} catch (Exception e) {}
		} else {
			// multi notice view
			nb.setContentTitle(currentlyNotifiedNotices.size() + " " + context.getString(R.string.notice_notification_multiple_header))
				.setNumber(currentlyNotifiedNotices.size())
				.setLargeIcon(BitmapFactory.decodeResource(context.getResources(), R.drawable.ic_stat_notify_boinc_normal))
				.setSmallIcon(R.drawable.mailw)
				.setContentIntent(contentIntent)
				.setSubText(context.getString(R.string.app_name));
			
			// append notice titles to list
			NotificationCompat.InboxStyle inboxStyle = new NotificationCompat.InboxStyle();
			for(Notice tmp: currentlyNotifiedNotices) {
				inboxStyle.addLine(tmp.project_name + ": " + tmp.title);
			}
			nb.setStyle(inboxStyle);
		}
		
		n = nb.build();
		
		return n;
	}
}
