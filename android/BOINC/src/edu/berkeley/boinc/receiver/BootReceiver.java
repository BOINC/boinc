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

import edu.berkeley.boinc.utils.*;

import edu.berkeley.boinc.AppPreferences;
import edu.berkeley.boinc.client.Monitor;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class BootReceiver extends BroadcastReceiver {  
	
    @Override
    public void onReceive(Context context, Intent intent) {
    	
    	AppPreferences prefs = new AppPreferences();
    	prefs.readPrefs(context);
    	
    	if(prefs.getAutostart()) {
    		if(Logging.DEBUG) Log.d(Logging.TAG,"BootReceiver autostart enabled, start Monitor...");
	    	Intent startServiceIntent = new Intent(context, Monitor.class);
	    	context.startService(startServiceIntent);
    	} else {
    		// do nothing
    		if(Logging.DEBUG) Log.d(Logging.TAG,"BootReceiver autostart disabeld - do nothing");
    	}
    	
    }
}

