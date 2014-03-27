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

package edu.berkeley.boinc.attach;

import java.util.ArrayList;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.*;
import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.attach.ProjectAttachService.ProjectAttachWrapper;
import android.app.Activity;
import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;

public class BatchProcessingActivity extends Activity{
	
	private ProjectAttachService attachService = null;
	private boolean asIsBound = false;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState);  
        if(Logging.DEBUG) Log.d(Logging.TAG, "BatchProcessingActivity onCreate"); 
		// setup layout
        setContentView(R.layout.attach_project_batch_processing_layout); 
        doBindService();
    }
    
	@Override
	protected void onDestroy() {
    	if(Logging.VERBOSE) Log.v(Logging.TAG, "BatchProcessingActivity onDestroy");
	    super.onDestroy();
    	doUnbindService();
	}	

	// triggered by continue button
	// button only visible if no conflicts occured.
	public void continueClicked(View v) {
        if(Logging.DEBUG) Log.d(Logging.TAG, "BatchProcessingActivity.continueClicked"); 
		// finally, start BOINCActivity
		Intent intent = new Intent(this, BOINCActivity.class);
		// add flags to return to main activity and clearing all others and clear the back stack
		intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP); 
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		intent.putExtra("targetFragment", R.string.tab_projects); // make activity display projects fragment
		startActivity(intent);
	}
	
	private ServiceConnection mASConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        // This is called when the connection with the service has been established, getService returns 
	    	// the Monitor object that is needed to call functions.
	        attachService = ((ProjectAttachService.LocalBinder)service).getService();
		    asIsBound = true;
		    
		    // start attaching projects
		    new AttachProjectAsyncTask().execute();
	    }

	    public void onServiceDisconnected(ComponentName className) {
	    	// This should not happen
	    	attachService = null;
	    	asIsBound = false;
	    }
	};
	
	private void doBindService() {
		// bind to attach service
		bindService(new Intent(this, ProjectAttachService.class), mASConnection, Service.BIND_AUTO_CREATE);
	}

	private void doUnbindService() {
	    if (asIsBound) {
	        // Detach existing connection.
	        unbindService(mASConnection);
	        asIsBound = false;
	    }
	}
	
	private class AttachProjectAsyncTask extends AsyncTask<Void, String, Void> {

		private int numberSelected;
		private int numberAttached;
		private int numberAttempted;
		
		@Override
		protected void onPreExecute() {
			numberSelected = attachService.getNumberSelectedProjects();
			if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectAsyncTask: " + numberSelected + " projects to attach....");
			((TextView) findViewById(R.id.status)).setText("contacting projects..."); //TODO // shown while project configs are loaded
			super.onPreExecute();
		}

		@Override
		protected Void doInBackground(Void... arg0) {
			// wait until service is ready
			while(!attachService.projectConfigRetrievalFinished) {
		    	if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectAsyncTask: project config retrieval has not finished yet, wait...");
		    	try{Thread.sleep(1000);} catch(Exception e){}
			}
	    	if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectAsyncTask: project config retrieval finished, continue with attach.");
			// attach projects, one at a time
			ArrayList<ProjectAttachWrapper> selectedProjects = attachService.getSelectedProjects();
			for(ProjectAttachWrapper selectedProject: selectedProjects) {
				if(selectedProject.result != ProjectAttachWrapper.RESULT_READY) continue; // skip already tried projects in batch processing
	    		publishProgress(selectedProject.info.name);
	    		int conflict = selectedProject.lookupAndAttach(false);
	    		if(conflict == ProjectAttachWrapper.RESULT_SUCCESS) numberAttached++;
	    		else if(Logging.ERROR) Log.e(Logging.TAG,"AttachProjectAsyncTask attach returned conflict: " + conflict);
	    	}
	    	if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectAsyncTask: finsihed.");
	    	return null;
		}
		
		@Override
		protected void onProgressUpdate(String... values) {
			numberAttempted++;
	    	if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectAsyncTask: trying: " + values[0]);
	    	((TextView) findViewById(R.id.status)).setText("" + numberAttempted + "/" + numberSelected + " Attaching " + values[0] + "...");
			super.onProgressUpdate(values);
		}

		@Override
		protected void onPostExecute(Void result) {
			boolean conflicts = attachService.unresolvedConflicts();
			if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectAsyncTask: conflicts? " + conflicts);
			
			if(conflicts) {
				if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectAsyncTask: conflicts exists, open resolution activity...");
				Intent intent = new Intent(BatchProcessingActivity.this, BatchConflictListActivity.class);
				intent.putExtra("conflicts", true);
				startActivity(intent);
			} else {
				// yeay!
				((ProgressBar) findViewById(R.id.pb)).setVisibility(View.INVISIBLE);
				((TextView) findViewById(R.id.status)).setText("selected: " + numberSelected + " ; attempted: " + numberAttempted + " ; attached: " + numberAttached);
				((Button) findViewById(R.id.continue_button)).setVisibility(View.VISIBLE);
			}
			super.onPostExecute(result);
		}
	}
}
