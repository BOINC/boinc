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

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.*;
import edu.berkeley.boinc.attach.ProjectAttachService.ProjectAttachWrapper;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

public class IndividualAttachActivity extends Activity{
	
	private ProjectAttachService attachService = null;
	private boolean asIsBound = false;
	
	private ProjectAttachWrapper project;
	
	private EditText emailET;
	private EditText nameET;
	private EditText pwdET;
	
	private ServiceConnection mASConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        // This is called when the connection with the service has been established, getService returns 
	    	// the Monitor object that is needed to call functions.
	        attachService = ((ProjectAttachService.LocalBinder)service).getService();
		    asIsBound = true;
		    
		    project = attachService.getNextSelectedProject();

			// set title
			TextView titleText = (TextView) findViewById(R.id.title);
			titleText.setText(project.name);
		    
			// set default input
		    ArrayList<String> values = attachService.getUserDefaultValues();
	        emailET.setText(values.get(0));
	        nameET.setText(values.get(1));
	        
	        new WaitForProjectInitializationAsync().execute();
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
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState); 
        doBindService();
        setContentView(R.layout.attach_project_individual_layout);
        emailET = (EditText) findViewById(R.id.email_input);
        nameET = (EditText) findViewById(R.id.name_input);
        pwdET = (EditText) findViewById(R.id.pwd_input);
    }

	@Override
	protected void onDestroy() {
	    doUnbindService();
	    super.onDestroy();
	}

	private class WaitForProjectInitializationAsync extends AsyncTask<Void, Void, Void> {

		@Override
		protected Void doInBackground(Void... params) {
			if(Logging.DEBUG) Log.d(Logging.TAG,"WaitForProjectInitializationAsync ....");
			
			// wait....
			while(project.result == ProjectAttachWrapper.RESULT_UNINITIALIZED) {
				// wait for config download to finish
				try{Thread.sleep(500);}catch(Exception e){}
			}
			
			return null;	
		}

		@Override
		protected void onPostExecute(Void result) {
			super.onPostExecute(result);
			
	    	if(Logging.DEBUG) Log.v(Logging.TAG, "IndividualAttachActivity.WaitForProjectInitializationAsync finished, result: " + project.result);

	        // find layout wrapper
			LinearLayout buttonWrapper = (LinearLayout) findViewById(R.id.button_wrapper);
			LinearLayout ongoingWrapper = (LinearLayout) findViewById(R.id.ongoing_wrapper);
			TextView statusText = (TextView) findViewById(R.id.status_text);
			
			// finalize layout
			if(project.result == ProjectAttachWrapper.RESULT_READY) {
				// project config download finished successfully, possible to continue with attach
				ongoingWrapper.setVisibility(View.GONE);
				statusText.setVisibility(View.GONE);
				buttonWrapper.setVisibility(View.VISIBLE);
				
				Button loginB = (Button) findViewById(R.id.login_button);
				loginB.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						if(Logging.DEBUG) Log.v(Logging.TAG, "IndividualAttachActivity login clicked");
						attachService.setCredentials(emailET.getText().toString(), nameET.getText().toString(), pwdET.getText().toString());
						Boolean[] param = new Boolean[1];
						param[0] = true;
						new ProjectAttachAsync().execute(param);
					}
				});
				Button registerB = (Button) findViewById(R.id.login_button);
				registerB.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						if(Logging.DEBUG) Log.v(Logging.TAG, "IndividualAttachActivity register clicked, disabled? " + project.config.clientAccountCreationDisabled);
						attachService.setCredentials(emailET.getText().toString(), nameET.getText().toString(), pwdET.getText().toString());
						if(project.config.clientAccountCreationDisabled) {
							// cannot register in client, open website
							Intent i = new Intent(Intent.ACTION_VIEW);
							i.setData(Uri.parse(project.config.masterUrl));
							startActivity(i);
						} else {
							// creation allowed...
							Boolean[] param = new Boolean[1];
							param[0] = false;
							new ProjectAttachAsync().execute(param);
						}
					}
				});
				Button forgotPwdB = (Button) findViewById(R.id.forgotpwd_button);
				forgotPwdB.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						if(Logging.DEBUG) Log.v(Logging.TAG, "IndividualAttachActivity forgot pwd clicked.");

						Intent i = new Intent(Intent.ACTION_VIEW);
						i.setData(Uri.parse(project.config.masterUrl + "/get_passwd.php"));
						startActivity(i);
					}
				});
				
			} else {
				// project config download failed, show error message
				ongoingWrapper.setVisibility(View.GONE);
				buttonWrapper.setVisibility(View.GONE);
				statusText.setVisibility(View.VISIBLE);
				statusText.setText(project.getResultDescription());
				//TODO offer continue button?! or show toast and finish?
			}
		}
	}
	
	private class ProjectAttachAsync extends AsyncTask<Boolean, Void, Integer> {
		
		@Override
		protected void onPreExecute() {
	        // find layout wrapper
			LinearLayout buttonWrapper = (LinearLayout) findViewById(R.id.button_wrapper);
			LinearLayout ongoingWrapper = (LinearLayout) findViewById(R.id.ongoing_wrapper);
			TextView statusText = (TextView) findViewById(R.id.status_text);

			statusText.setVisibility(View.GONE);
			buttonWrapper.setVisibility(View.GONE);
			ongoingWrapper.setVisibility(View.VISIBLE);
			
			TextView ongoingText = (TextView) findViewById(R.id.ongoing_desc);
			ongoingText.setText(getString(R.string.attachproject_working_attaching) + " " + project.name);
			
			super.onPreExecute();
		}

		@Override
		protected Integer doInBackground(Boolean... params) {
			if(Logging.DEBUG) Log.d(Logging.TAG,"ProjectAttachAsync, login: " + params[0]);
			
			if(asIsBound) {
				return project.lookupAndAttach(params[0]);
			} else if(Logging.WARNING) Log.w(Logging.TAG,"ProjectAttachAsync, do nothing, not bound.");
			
			return ProjectAttachWrapper.RESULT_UNDEFINED;	
		}

		@SuppressLint("NewApi")
		@Override
		protected void onPostExecute(Integer result) {
			super.onPostExecute(result);
			
	    	if(Logging.DEBUG) Log.v(Logging.TAG, "IndividualAttachActivity.ProjectAttachAsync finished, result: " + result);
			
			if(result == ProjectAttachWrapper.RESULT_SUCCESS) {
				// successful
				if(attachService.getNextSelectedProject() == null) {
					// all successful, return to BOINCActivity
					Intent intent = new Intent(getApplicationContext(), BOINCActivity.class);
					// add flags to return to main activity and clearing all others and clear the back stack
					intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP); 
					intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
					intent.putExtra("targetFragment", R.string.tab_projects); // make activity display projects fragment
					startActivity(intent);
				} else {
					// there are more selected projects that need to be attached, restart activity
					if (Build.VERSION.SDK_INT >= 11) {
					    recreate();
					} else {
					    Intent intent = getIntent();
					    intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
					    finish();
					    overridePendingTransition(0, 0);

					    startActivity(intent);
					    overridePendingTransition(0, 0);
					}
				}
			} else {
				// something went wrong with this attach
		        // find layout wrapper
				LinearLayout buttonWrapper = (LinearLayout) findViewById(R.id.button_wrapper);
				LinearLayout ongoingWrapper = (LinearLayout) findViewById(R.id.ongoing_wrapper);
				TextView statusText = (TextView) findViewById(R.id.status_text);

				ongoingWrapper.setVisibility(View.GONE);
				statusText.setVisibility(View.VISIBLE);
				statusText.setText(project.getResultDescription());
				buttonWrapper.setVisibility(View.VISIBLE);
			}
		}
	}
	
}
