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
package edu.berkeley.boinc;

import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import android.app.TabActivity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.Resources;
import android.os.Bundle; 
import android.os.IBinder;
import android.util.Log;  
import android.widget.TabHost;
import android.widget.TabHost.TabSpec;
import android.widget.Toast;

public class AndroidBOINCActivity extends TabActivity {
	
	private final String TAG = "AndroidBOINCActivity"; 
	
	private Monitor monitor;
	public static ClientStatus client;
	
	private Boolean mIsBound;

	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        // This is called when the connection with the service has been established, getService returns the Monitor object that is needed to call functions.
	        monitor = ((Monitor.LocalBinder)service).getService();
	    }

	    public void onServiceDisconnected(ComponentName className) {
	        // This should not happen
	        monitor = null;
	        Toast.makeText(getApplicationContext(), "service disconnected", Toast.LENGTH_SHORT).show();
	    }
	};

	private void doBindService() {
		// Service has to be started "sticky" by the first instance that uses it. It causes the service to stay around, even when all Activities are destroyed (on purpose or by the system)
		// check whether service already started by BootReceiver is done within the service.
		startService(new Intent(this,Monitor.class));
		
	    // Establish a connection with the service, onServiceConnected gets called when
		bindService(new Intent(this, Monitor.class), mConnection, 0);
	    mIsBound = true;
	}

	private void doUnbindService() {
	    if (mIsBound) {
	        // Detach existing connection.
	        unbindService(mConnection);
	        mIsBound = false;
	    }
	}

	@Override
	protected void onDestroy() {
    	Log.d(TAG, "onDestroy");
	    doUnbindService();
	    super.onDestroy();
	}
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState);  
        setContentView(R.layout.main);  
         
        Log.d(TAG, "onCreate"); 
        
        //bind monitor service
        doBindService();
        
        //setup tab layout
        setupTabLayout();
    }
    
    public static void logMessage(Context ctx, String tag, String message) {
        Intent testLog = new Intent();
        testLog.setAction("edu.berkeley.boinc.log");
        testLog.putExtra("message", message);   
        testLog.putExtra("tag", tag);
        ctx.sendBroadcast(testLog);
    }
    
    /*
     * setup tab layout.
     * which tabs should be set up is defined in resources file: /res/values/configuration.xml
     */
    private void setupTabLayout() {
    	
    	Resources res = getResources();
    	TabHost tabHost = getTabHost();
        
    	if(res.getBoolean(R.bool.tab_status)) {
	        TabSpec statusSpec = tabHost.newTabSpec(getResources().getString(R.string.tab_status));
	        statusSpec.setIndicator(getResources().getString(R.string.tab_status), getResources().getDrawable(R.drawable.icon_status_tab));
	        Intent statusIntent = new Intent(this,StatusActivity.class);
	        statusSpec.setContent(statusIntent);
	        tabHost.addTab(statusSpec);
    	}
        
    	if(res.getBoolean(R.bool.tab_tasks)) {
	        TabSpec tasksSpec = tabHost.newTabSpec(getResources().getString(R.string.tab_tasks));
	        tasksSpec.setIndicator(getResources().getString(R.string.tab_tasks), getResources().getDrawable(R.drawable.icon_tasks_tab));
	        Intent tasksIntent = new Intent(this,TasksActivity.class);
	        tasksSpec.setContent(tasksIntent);
	        tabHost.addTab(tasksSpec);
    	}
        
    	if(res.getBoolean(R.bool.tab_transfers)) {
	        TabSpec transSpec = tabHost.newTabSpec(getResources().getString(R.string.tab_transfers));
	        transSpec.setIndicator(getResources().getString(R.string.tab_transfers), getResources().getDrawable(R.drawable.icon_trans_tab));
	        Intent transIntent = new Intent(this,TransActivity.class);
	        transSpec.setContent(transIntent);
	        tabHost.addTab(transSpec);
    	}
        
    	if(res.getBoolean(R.bool.tab_preferences)) {
	        TabSpec prefsSpec = tabHost.newTabSpec(getResources().getString(R.string.tab_preferences));
	        prefsSpec.setIndicator(getResources().getString(R.string.tab_preferences), getResources().getDrawable(R.drawable.icon_prefs_tab));
	        Intent prefsIntent = new Intent(this,PrefsActivity.class);
	        prefsSpec.setContent(prefsIntent);
	        tabHost.addTab(prefsSpec);
    	}
        
    	if(res.getBoolean(R.bool.tab_messages)) {
	        TabSpec msgsSpec = tabHost.newTabSpec(getResources().getString(R.string.tab_messages));
	        msgsSpec.setIndicator(getResources().getString(R.string.tab_messages), getResources().getDrawable(R.drawable.icon_msgs_tab));
	        Intent msgsIntent = new Intent(this,MsgsActivity.class);
	        msgsSpec.setContent(msgsIntent);
	        tabHost.addTab(msgsSpec);
    	}
        
    	if(res.getBoolean(R.bool.tab_debug)) {
	        TabSpec debugSpec = tabHost.newTabSpec(getResources().getString(R.string.tab_debug));
	        debugSpec.setIndicator(getResources().getString(R.string.tab_debug), getResources().getDrawable(R.drawable.icon_debug_tab));
	        Intent debugIntent = new Intent(this,DebugActivity.class);
	        debugSpec.setContent(debugIntent);
	        tabHost.addTab(debugSpec);
        }
    	
        Log.d(TAG,"tab layout setup done");

        AndroidBOINCActivity.logMessage(this, TAG, "tab setup finished");
    }
}
