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
package edu.berkeley.boinc.client;

import java.util.ArrayList;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import edu.berkeley.boinc.MainActivity;
import edu.berkeley.boinc.definitions.CommonDefs;
import edu.berkeley.boinc.rpc.CcStatus;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.Message;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.Result;
import edu.berkeley.boinc.rpc.Transfer;

/*
 * Singleton that holds the client status data, as determined by the Monitor.
 * To get instance call Monitor.getClientStatus()
 */
public class ClientStatus {
	
	private final String TAG = "ClientStatus";
	private Context ctx; // application context in order to fire broadcast events
	
	//RPC wrapper
	private CcStatus status;
	private ArrayList<Result> results;
	private ArrayList<Project> projects;
	private ArrayList<Transfer> transfers;
	private GlobalPreferences prefs;
	private ArrayList<Message> messages; //debug tab
	
	// setup status
	public Integer setupStatus = 0;
	public static final int SETUP_STATUS_LAUNCHING = 0; // 0 = client is in setup routine (default)
	public static final int SETUP_STATUS_AVAILABLE = 1; // 1 = client is launched and available for RPC (connected and authorized)
	public static final int SETUP_STATUS_ERROR = 2; // 2 = client is in a permanent error state
	public static final int SETUP_STATUS_NOPROJECT = 3; // 3 = client is launched but not attached to the project (login)
	private Boolean setupStatusParseError = false;
	
	// computing status
	public Integer computingStatus = 2;
	public static final int COMPUTING_STATUS_NEVER = 0;
	public static final int COMPUTING_STATUS_SUSPENDED = 1;
	public static final int COMPUTING_STATUS_IDLE = 2;
	public static final int COMPUTING_STATUS_COMPUTING = 3;
	public Integer computingSuspendReason = 0; //reason why computing got suspended, only if COMPUTING_STATUS_SUSPENDED
	private Boolean computingParseError = false; //indicates that status could not be parsed and is therefore invalid
	
	//network status
	public Integer networkStatus = 2;
	public static final int NETWORK_STATUS_NEVER = 0;
	public static final int NETWORK_STATUS_SUSPENDED = 1;
	public static final int NETWORK_STATUS_AVAILABLE = 2;
	public Integer networkSuspendReason = 0; //reason why network activity got suspended, only if NETWORK_STATUS_SUSPENDED
	private Boolean networkParseError = false; //indicates that status could not be parsed and is therefore invalid
	
	/*
	 * fires "clientstatuschange" broadcast, so registered Activities can update their model.
	 */
	public synchronized void fire() {
		if(ctx!=null) {
	        Intent clientChanged = new Intent();
	        clientChanged.setAction("edu.berkeley.boinc.clientstatuschange");
			ctx.sendBroadcast(clientChanged,null);
		}else {
			Log.d(TAG,"cant fire, not context set!");
		}
	}
	
	/*
	 * Application context is required by the broadcast mechanism, reference is copied by Monitor service on start up.
	 */
	public synchronized void setCtx(Context tctx) {
		this.ctx = tctx;
	}
	
	/*
	 * called frequently by Monitor to set the RPC data. These objects are used to determine the client status and parse it in the data model of this class.
	 */
	public synchronized void setClientStatus(CcStatus status,ArrayList<Result> results,ArrayList<Project> projects, ArrayList<Transfer> transfers, GlobalPreferences clientPrefs, ArrayList<Message> msgs) {
		this.status = status;
		this.results = results;
		this.projects = projects;
		this.transfers = transfers;
		this.prefs = clientPrefs;
		this.messages = msgs;
		parseClientStatus();
		Log.d(TAG,"parsing results: computing: " + computingParseError + computingStatus + computingSuspendReason + " - network: " + networkParseError + networkStatus + networkSuspendReason);
		if(!computingParseError && !networkParseError && !setupStatusParseError) {
			fire(); // broadcast that status has changed
		} else {
			MainActivity.logMessage(ctx, TAG, "discard status change due to parse error" + computingParseError + computingStatus + computingSuspendReason + "-" + networkParseError + networkStatus + networkSuspendReason + "-" + setupStatusParseError);
		}
	}
	
	public synchronized ArrayList<Result> getTasks() {
		if(results == null) { //check in case monitor is not set up yet (e.g. while logging in)
			Log.d(TAG, "state is null");
			return null;
		}
		return results;
	}
	
	public synchronized ArrayList<Transfer> getTransfers() {
		if(transfers == null) { //check in case monitor is not set up yet (e.g. while logging in)
			Log.d(TAG, "transfers is null");
			return null;
		}
		return transfers;
	}
	
	public synchronized GlobalPreferences getPrefs() {
		if(prefs == null) { //check in case monitor is not set up yet (e.g. while logging in)
			Log.d(TAG, "prefs is null");
			return null;
		}
		return prefs;
	}
	
	public synchronized ArrayList<Project> getProjects() {
		if(projects == null) { //check in case monitor is not set up yet (e.g. while logging in)
			Log.d(TAG, "getProject() state is null");
			return null;
		}
		return projects;
	}
	
	//Debug Tab
	public synchronized ArrayList<Message> getMessages() {
		if(messages == null) { //check in case monitor is not set up yet (e.g. while logging in)
			Log.d(TAG, "messages is null");
			return null;
		}
		return messages;
	}
	
	public synchronized void resetMessages() {
		if(messages == null) { //check in case monitor is not set up yet (e.g. while logging in)
			Log.d(TAG, "messages is null");
			return;
		}
		messages.clear();
	}
	
	/*
	 * parses RPC data to ClientStatus data model.
	 */
	private void parseClientStatus() {
		parseComputingStatus();
		parseProjectStatus();
		parseNetworkStatus();
	}
	
	private void parseProjectStatus() {
		try {
			if (projects.size() > 0) { 
				setupStatus = SETUP_STATUS_AVAILABLE;
				setupStatusParseError = false;
			} else { //not projects attached
				setupStatus = SETUP_STATUS_NOPROJECT;
				setupStatusParseError = false;
			}
		} catch (Exception e) {
			setupStatusParseError = true;
			Log.e(TAG, "parseProjectStatus - Exception", e);
			MainActivity.logMessage(ctx, TAG, "error parsing setup status (project state)");
		}
	}
	
	private void parseComputingStatus() {
		computingParseError = true;
		try {
			if(status.task_mode==CommonDefs.RUN_MODE_NEVER) {
				computingStatus = COMPUTING_STATUS_NEVER;
				computingSuspendReason = status.task_suspend_reason; // = 4 - SUSPEND_REASON_USER_REQ????
				computingParseError = false;
				return;
			}
			if((status.task_mode == CommonDefs.RUN_MODE_AUTO) && (status.task_suspend_reason != CommonDefs.SUSPEND_NOT_SUSPENDED)) {
				computingStatus = COMPUTING_STATUS_SUSPENDED;
				computingSuspendReason = status.task_suspend_reason;
				computingParseError = false;
				return;
			}
			if((status.task_mode == CommonDefs.RUN_MODE_AUTO) && (status.task_suspend_reason == CommonDefs.SUSPEND_NOT_SUSPENDED)) {
				//figure out whether we have an active task
				Boolean activeTask = false;
				if(results!=null) {
					for(Result task: results) {
						if(task.active_task) { // this result has corresponding "active task" in RPC XML
							activeTask = true;
							continue; // amount of active tasks does not matter.
						}
					}
				}
				
				if(activeTask) { // client is currently computing
					computingStatus = COMPUTING_STATUS_COMPUTING;
					computingSuspendReason = status.task_suspend_reason; // = 0 - SUSPEND_NOT_SUSPENDED
					computingParseError = false;
					return;
				} else { // client "is able but idle"
					computingStatus = COMPUTING_STATUS_IDLE;
					computingSuspendReason = status.task_suspend_reason; // = 0 - SUSPEND_NOT_SUSPENDED
					computingParseError = false;
					return;
				}
			}
		} catch (Exception e) {
			Log.e(TAG, "parseComputingStatus - Exception", e);
			MainActivity.logMessage(ctx, TAG, "error - client computing status");
		}
	}
	
	private void parseNetworkStatus() {
		networkParseError = true;
		try {
			if(status.network_mode==CommonDefs.RUN_MODE_NEVER) {
				networkStatus = NETWORK_STATUS_NEVER;
				networkSuspendReason = status.network_suspend_reason; // = 4 - SUSPEND_REASON_USER_REQ????
				networkParseError = false;
				return;
			}
			if((status.network_mode == CommonDefs.RUN_MODE_AUTO) && (status.network_suspend_reason != CommonDefs.SUSPEND_NOT_SUSPENDED)) {
				networkStatus = NETWORK_STATUS_SUSPENDED;
				networkSuspendReason = status.network_suspend_reason;
				networkParseError = false;
				return;
			}
			if((status.network_mode == CommonDefs.RUN_MODE_AUTO) && (status.network_suspend_reason == CommonDefs.SUSPEND_NOT_SUSPENDED)) {
				networkStatus = NETWORK_STATUS_AVAILABLE;
				networkSuspendReason = status.network_suspend_reason; // = 0 - SUSPEND_NOT_SUSPENDED
				networkParseError = false;
				return;
			}
		} catch (Exception e) {
			Log.e(TAG, "parseNetworkStatus - Exception", e);
			MainActivity.logMessage(ctx, TAG, "error - client network status");
		}
	}
	
}
