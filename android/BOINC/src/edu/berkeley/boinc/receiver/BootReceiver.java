/*******************************************************************************
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2012 University of California
 * 
 * BOINC is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * BOINC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/
package edu.berkeley.boinc.receiver;

import edu.berkeley.boinc.AppPreferences;
import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.client.Monitor;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class BootReceiver extends BroadcastReceiver {  
	
	private final String TAG = "BOINC BootReceiver";
	private NotificationManager mNM;
	
    @Override
    public void onReceive(Context context, Intent intent) {
    	
    	AppPreferences prefs = new AppPreferences();
    	prefs.readPrefs(context);
    	
    	if(prefs.getAutostart()) {
    		Log.d(TAG,"autostart enabled, start Monitor...");
	    	Intent startServiceIntent = new Intent(context, Monitor.class);
	    	//startServiceIntent.putExtra("autostart", true);
	    	context.startService(startServiceIntent);
	    	
			mNM = (NotificationManager) context.getSystemService(Service.NOTIFICATION_SERVICE);
	        Notification notification = new Notification(R.drawable.boinc, context.getString(R.string.autostart_notification_header), System.currentTimeMillis());
	        PendingIntent contentIntent = PendingIntent.getActivity(context.getApplicationContext(), 0, new Intent(context.getApplicationContext(), BOINCActivity.class), 0);

	        // Set current view for notification panel
	        notification.setLatestEventInfo(context.getApplicationContext(), context.getString(R.string.autostart_notification_header), context.getString(R.string.autostart_notification_text), contentIntent);

	        // Send notification
	        mNM.notify(context.getResources().getInteger(R.integer.autostart_notification_id), notification);
    	} else {
    		// do nothing
    		Log.d(TAG,"autostart disabeld");
    	}
    	
    }
}

