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

import edu.berkeley.boinc.utils.*;

import java.util.ArrayList;
import edu.berkeley.boinc.adapter.AttachProjectListAdapter;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.ProjectInfo;
import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class AttachProjectListActivity extends Activity implements android.view.View.OnClickListener{

	private ListView lv;
	private AttachProjectListAdapter listAdapter;
	private Dialog manualUrlInputDialog;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState);  
        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
         
        if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectListActivity onCreate"); 
        
		//get supported projects
		// try to get current client status from monitor
		ClientStatus status;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"AttachProjectListActivity: Could not load supported projects, clientStatus not initialized.");
			finish();
			return;
		}
		ArrayList<ProjectInfo> data = status.supportedProjects;
		if(Logging.DEBUG) Log.d(Logging.TAG,"monitor.getAndroidProjectsList returned with " + data.size() + " elements");
		
		// setup layout
        setContentView(R.layout.attach_project_list_layout);  
		lv = (ListView) findViewById(R.id.listview);
        listAdapter = new AttachProjectListAdapter(AttachProjectListActivity.this,R.id.listview,data);
        lv.setAdapter(listAdapter);
        
        // set title bar
        getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.title_bar);
    }
    
	@Override
	protected void onDestroy() {
    	if(Logging.VERBOSE) Log.v(Logging.TAG, "AttachProjectListActivity onDestroy");
	    super.onDestroy();
	}
	
	// check whether device is online before starting connection attempt
	// as needed for AttachProjectLoginActivity (retrieval of ProjectConfig)
	// note: available internet does not imply connection to project server
	// is possible!
	private Boolean checkDeviceOnline() {
	    ConnectivityManager connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
	    NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
	    return activeNetworkInfo != null && activeNetworkInfo.isConnectedOrConnecting();
	}
	
	// get called by manual input list item
	public void manualUrlItem(View view) {
		//if(Logging.DEBUG) Log.d(Logging.TAG,"manualUrlItem");
		//show dialog
		showDialog(view.getId());
	}
	
	// gets called by showDialog
	@Override
	protected Dialog onCreateDialog(int id) {
		manualUrlInputDialog = new Dialog(this); //instance new dialog
		manualUrlInputDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
		manualUrlInputDialog.setContentView(R.layout.attach_project_list_layout_manual_dialog);
		Button button = (Button) manualUrlInputDialog.findViewById(R.id.buttonUrlSubmit);
		button.setOnClickListener(this);
		((TextView)manualUrlInputDialog.findViewById(R.id.title)).setText(R.string.attachproject_list_manual_dialog_title);
		return manualUrlInputDialog;
	}

	// gets called by dialog button
	@Override
	public void onClick(View v) {
		//if(Logging.DEBUG) Log.d(Logging.TAG,"buttonUrlSubmit clicked");
		try {
			String url = ((EditText)manualUrlInputDialog.findViewById(R.id.Input)).getText().toString();

			if(url == null) { // error while parsing
				showErrorToast(R.string.attachproject_list_manual_no_url);
			}
			else if(url.length()== 0) { //nothing in edittext
				showErrorToast(R.string.attachproject_list_manual_no_url);
			}
			else if(!checkDeviceOnline()) {
				showErrorToast(R.string.attachproject_list_no_internet);
			} else {
				manualUrlInputDialog.dismiss();
				startAttachProjectLoginActivity(null, url);
			}
		} catch (Exception e) {
			if(Logging.WARNING) Log.w(Logging.TAG,"error parsing edit text",e);
		}
	}
	
	// gets called by project list item
	public void onProjectClick(View view) {
		//if(Logging.DEBUG) Log.d(Logging.TAG,"onProjectClick");
		if(!checkDeviceOnline()) {
			showErrorToast(R.string.attachproject_list_no_internet);
			return;
		}
		try {
			ProjectInfo project = (ProjectInfo) view.getTag();
			startAttachProjectLoginActivity(project, null); 
		} catch (Exception e) {
			if(Logging.WARNING) Log.w(Logging.TAG,"error parsing view tag",e);
			showErrorToast(R.string.attachproject_list_manual_no_url);
		}
	}
	
	private void startAttachProjectLoginActivity(ProjectInfo project, String url) {
		//if(Logging.DEBUG) Log.d(Logging.TAG,"startAttachProjectLoginActivity ");
		Intent intent = new Intent(this, AttachProjectLoginActivity.class);
		intent.putExtra("projectInfo", project);
		intent.putExtra("url", url);
		startActivity(intent);
	}

	private void showErrorToast(int resourceId) {
		Toast toast = Toast.makeText(getApplicationContext(), resourceId, Toast.LENGTH_SHORT);
		toast.show();
	}

}
