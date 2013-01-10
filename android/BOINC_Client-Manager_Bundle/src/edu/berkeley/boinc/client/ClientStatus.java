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

import edu.berkeley.boinc.AndroidBOINCActivity;
import edu.berkeley.boinc.definitions.CommonDefs;
import edu.berkeley.boinc.rpc.CcState;
import edu.berkeley.boinc.rpc.CcStatus;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.Message;
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
	private CcState state;
	private ArrayList<Transfer> transfers;
	private GlobalPreferences prefs;
	private ArrayList<Message> messages; //debug tab
	
	//setup status, set by "setupClient" method of ClientMonitorAsync
	// 0 = client is in setup routine (default)
	// 1 = client is launched and available for RPC (connected and authorized)
	// 2 = client is in a permanent error state, there are not attempts to fix it (otherwise 0)
	// 3 = client is launched but not attached to the project (login)
	public Integer setupStatus = 0;
	
	//computing status
	public Integer computingStatus = 2;
	// 0 = client run mode is NEVER "stopped by user"
	// 1 = client run mode is AUTO, but suspended due to certain reason (->computingSuspendReason)
	// 2 = client run mode is AUTO, client is not suspended but idle (default)
	// 3 = client run mode is AUTO, client is computing
	public Integer computingSuspendReason = 0; //reason why computing got suspended
	private Boolean computingParseError = false; //indicates that status could not be parsed and is therefore invalid
	
	//network status
	public Integer networkStatus = 2;
	// 0 = network run mode is NEVER "disabled by user"
	// 1 = network run mode is AUTO, but suspended due to certain reason (->networkSuspendReason)
	// 2 = network run mode is AUTO and not suspended (default)
	public Integer networkSuspendReason = 0; //reason why network activity got suspended
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
	public synchronized void setClientStatus(CcStatus status,CcState state, ArrayList<Transfer> transfers, GlobalPreferences clientPrefs, ArrayList<Message> msgs) {
		this.status = status;
		this.state = state;
		this.transfers = transfers;
		this.prefs = clientPrefs;
		this.messages = msgs;
		parseClientStatus();
		Log.d(TAG,"parsing results: computing: " + computingParseError + computingStatus + computingSuspendReason + " - network: " + networkParseError + networkStatus + networkSuspendReason);
		if(!computingParseError && !networkParseError) {
			fire(); // broadcast that status has changed
		} else {
			AndroidBOINCActivity.logMessage(ctx, TAG, "discard status change due to parse error" + computingParseError + computingStatus + computingSuspendReason + "-" + networkParseError + networkStatus + networkSuspendReason);
		}
	}
	
	public synchronized ArrayList<Result> getTasks() {
		if(state == null) { //check in case monitor is not set up yet (e.g. while logging in)
			Log.d(TAG, "state is null");
			return null;
		}
		return state.results;
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
		parseNetworkStatus();
	}
	
	private void parseComputingStatus() {
		computingParseError = true;
		try {
			if(status.task_mode==CommonDefs.RUN_MODE_NEVER) {
				computingStatus = 0;
				computingSuspendReason = status.task_suspend_reason; // = 4 - SUSPEND_REASON_USER_REQ????
				computingParseError = false;
				return;
			}
			if((status.task_mode == CommonDefs.RUN_MODE_AUTO) && (status.task_suspend_reason != CommonDefs.SUSPEND_NOT_SUSPENDED)) {
				computingStatus = 1;
				computingSuspendReason = status.task_suspend_reason;
				computingParseError = false;
				return;
			}
			if((status.task_mode == CommonDefs.RUN_MODE_AUTO) && (status.task_suspend_reason == CommonDefs.SUSPEND_NOT_SUSPENDED)) {
				//figure out whether we have an active task
				Boolean activeTask = false;
				if(state.results!=null) {
					for(Result task: state.results) {
						if(task.active_task) { // this result has corresponding "active task" in RPC XML
							activeTask = true;
							continue; // amount of active tasks does not matter.
						}
					}
				}
				
				if(activeTask) { // client is currently computing
					computingStatus = 3;
					computingSuspendReason = status.task_suspend_reason; // = 0 - SUSPEND_NOT_SUSPENDED
					computingParseError = false;
					return;
				} else { // client "is able but idle"
					computingStatus = 2;
					computingSuspendReason = status.task_suspend_reason; // = 0 - SUSPEND_NOT_SUSPENDED
					computingParseError = false;
					return;
				}
			}
		} catch (Exception e) {
			Log.e(TAG, "parseComputingStatus - Exception", e);
			AndroidBOINCActivity.logMessage(ctx, TAG, "error - client computing status");
		}
	}
	
	private void parseNetworkStatus() {
		networkParseError = true;
		try {
			if(status.network_mode==CommonDefs.RUN_MODE_NEVER) {
				networkStatus = 0;
				networkSuspendReason = status.network_suspend_reason; // = 4 - SUSPEND_REASON_USER_REQ????
				networkParseError = false;
				return;
			}
			if((status.network_mode == CommonDefs.RUN_MODE_AUTO) && (status.network_suspend_reason != CommonDefs.SUSPEND_NOT_SUSPENDED)) {
				networkStatus = 1;
				networkSuspendReason = status.network_suspend_reason;
				networkParseError = false;
				return;
			}
			if((status.network_mode == CommonDefs.RUN_MODE_AUTO) && (status.network_suspend_reason == CommonDefs.SUSPEND_NOT_SUSPENDED)) {
				networkStatus = 2;
				networkSuspendReason = status.network_suspend_reason; // = 0 - SUSPEND_NOT_SUSPENDED
				networkParseError = false;
				return;
			}
		} catch (Exception e) {
			Log.e(TAG, "parseNetworkStatus - Exception", e);
			AndroidBOINCActivity.logMessage(ctx, TAG, "error - client network status");
		}
	}
	
}
