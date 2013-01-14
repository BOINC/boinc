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

import java.util.ArrayList;

import edu.berkeley.boinc.adapter.PrefsListAdapter;
import edu.berkeley.boinc.adapter.PrefsListItemWrapper;
import edu.berkeley.boinc.adapter.PrefsListItemWrapperBool;
import edu.berkeley.boinc.adapter.PrefsListItemWrapperDouble;
import edu.berkeley.boinc.adapter.PrefsListItemWrapperText;
import edu.berkeley.boinc.adapter.ProjectsListAdapter;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.Result;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.FragmentActivity;
import android.text.InputType;
import android.text.method.PasswordTransformationMethod;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

public class ProjectsActivity extends FragmentActivity {
	
	private final String TAG = "BOINC ProjectsActivity";
	
	private Monitor monitor;
	private Boolean mIsBound = false;
	
	private ListView lv;
	private ProjectsListAdapter listAdapter;
	
	private ArrayList<Project> data = new ArrayList<Project>(); //Adapter for list data
	
	private Boolean dataOutdated = false; //shows that current data is out of date. this happens when writing client status async just started and has not returned (clientstatechanged broadcast) yet.
	private Boolean initialSetup = true; //prevents reinitPrefsLayout() from adding elements to the list, every time it gets called. also prevent loadLayout from reading elements out of bounds (array not set up, if this is not false)
	
	/*
	 * Receiver is necessary, because writing of prefs has to be done asynchroneously. PrefsActivity will change to "loading" layout, until monitor read new results.
	 */
	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context,Intent intent) {
			Log.d(TAG+"-localClientStatusRecNoisy","received");
			if(dataOutdated) { //cause activity to re-init layout
				Log.d(TAG, "data was outdated, go directly to reinitPrefsLayout");
				dataOutdated = false;
				reinitProjectsLayout();
			}
			loadProjects();
		}
	};
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		doBindService();
	}
	
	public void onResume() {
		super.onResume();
		//gets called every time Activity comes to front, therefore also after onCreate
		
		//register receiver of client status
		registerReceiver(mClientStatusChangeRec,ifcsc);
		
		//determine layout
		if(initialSetup) { //no data available (first call) 
			setContentView(R.layout.projects_layout_loading);
		}
		
		loadProjects();
	}
	
	public void onPause() {
		//unregister receiver, so there are not multiple intents flying in
		Log.d(TAG+"-onPause","remove receiver");
		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}
	
	/*
	 * Service binding part
	 * only necessary, when function on monitor instance has to be called
	 * currently in Prefs- and DebugActivity 
	 * 
	 */
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	    	Log.d(TAG,"onServiceConnected");
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
			loadProjects();
	    }

	    public void onServiceDisconnected(ComponentName className) {
	        monitor = null;
	        mIsBound = false;
	    }
	};

	private void doBindService() {
		if(!mIsBound) {
			getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, 0); //calling within Tab needs getApplicationContext() for bindService to work!
		}
	}

	private void doUnbindService() {
	    if (mIsBound) {
	    	getApplicationContext().unbindService(mConnection);
	        mIsBound = false;
	    }
	}
	
	private void loadProjects() {
		ArrayList<Project> tmpA = Monitor.getClientStatus().getProjects(); //read project from state saved in ClientStatus
		if(tmpA == null) {
			Log.d(TAG, "loadProjects returns, data is not present");
			return;
		}
		
		if(initialSetup) {
			//init layout instead
			reinitProjectsLayout();
		} else {
			Log.d(TAG, "loadProjects outdated: " + dataOutdated );
			if(dataOutdated) { //data is not present or not current, show loading instead!
				setContentView(R.layout.projects_layout_loading);
			} else {
				//deep copy, so ArrayList adapter actually recognizes the difference
				data.clear();
				for (Project tmp: tmpA) {
					data.add(tmp);
				}
				listAdapter.notifyDataSetChanged(); //force list adapter to refresh
			}
		}
	}
	
	private void reinitProjectsLayout() {
		
		setContentView(R.layout.projects_layout);
		lv = (ListView) findViewById(R.id.listview);
        listAdapter = new ProjectsListAdapter(ProjectsActivity.this,R.id.listview,data);
        lv.setAdapter(listAdapter);

        if(initialSetup) { //prevent from re-population when reinit is called after dataOutdated
        	initialSetup = false;
        }
	}
	
	// handler for onClick of listItem
	public void onItemClick (View view) {
		Project project = (Project) view.getTag(); //gets added to view by ProjectsListAdapter
		Log.d(TAG,"onItemClick projectName: " + project.project_name + " - url: " + project.master_url);
		(new ConfirmDeletionDialogFragment(project.project_name, project.master_url)).show(getSupportFragmentManager(), "confirm_projects_deletion");
	}

	@Override
	protected void onDestroy() {
	    Log.d(TAG,"onDestroy()");
	    super.onDestroy();
	    doUnbindService();
	}
	
	public class ConfirmDeletionDialogFragment extends DialogFragment {
		
		private final String TAG = "ConfirmDeletionDialogFragment";
		
		private String name = "";
		private String url;
		
		public ConfirmDeletionDialogFragment(String name, String url) {
			this.name = name;
			this.url = url;
		}
		
	    @Override
	    public Dialog onCreateDialog(Bundle savedInstanceState) {
	        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
	        String dialogTitle = getString(R.string.confirm_deletion) + " " + name + "?";
	        builder.setMessage(dialogTitle)
	               .setPositiveButton(R.string.confirm_deletion_confirm, new DialogInterface.OnClickListener() {
	                   public void onClick(DialogInterface dialog, int id) {
	                       Log.d(TAG,"confirm clicked.");
	                       monitor.detachProjectAsync(url); //asynchronous call to detach project with given url.
	                       dataOutdated = true; //async call started, data out dated until broadcast
	                   }
	               })
	               .setNegativeButton(R.string.confirm_deletion_cancel, new DialogInterface.OnClickListener() {
	                   public void onClick(DialogInterface dialog, int id) {
	                       Log.d(TAG,"dialog canceled.");
	                   }
	               });
	        // Create the AlertDialog object and return it
	        return builder.create();
	    }
	}

}
