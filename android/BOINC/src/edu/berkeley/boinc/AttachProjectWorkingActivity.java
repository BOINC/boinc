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

import edu.berkeley.boinc.client.Monitor;
import android.app.Activity;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.utils.BOINCErrors;

public class AttachProjectWorkingActivity extends Activity{
	
	private Monitor monitor;
	private Boolean mIsBound = false;
	
	private Integer timeInterval;
	
	private ArrayList<View> views = new ArrayList<View>();
	private ViewGroup anchor;
	
	private Boolean registration; // if false, login attempt
	private String projectUrl;
	private String projectName;
	private String id;
	private String userName;
	private String teamName;
	private String eMail;
	private String pwd;
	private Boolean usesName;
	
	
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        // This is called when the connection with the service has been established, getService returns the Monitor object that is needed to call functions.
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;        
		    
		    // do desired action
		    new ProjectAccountAsync(registration, projectUrl, id, eMail, userName, teamName, pwd, usesName, projectName).execute();
	    }

	    public void onServiceDisconnected(ComponentName className) { // This should not happen
	        monitor = null;
		    mIsBound = false;
	    }
	};
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState);  
        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
        
        // bind monitor service
        doBindService();

    	//parse information from intent extras
        try {
        	registration = getIntent().getBooleanExtra("registration", false);
        	usesName = getIntent().getBooleanExtra("usesName", false);
        	projectUrl = getIntent().getStringExtra("projectUrl");
        	projectName = getIntent().getStringExtra("projectName");
        	userName = getIntent().getStringExtra("userName");
        	teamName = getIntent().getStringExtra("teamName");
        	eMail = getIntent().getStringExtra("eMail");
        	pwd = getIntent().getStringExtra("pwd");
        	id = getIntent().getStringExtra("id");
        			
        	if(Logging.DEBUG) Log.d(Logging.TAG,"AttachProjectWorkingActivity intent extras: " + projectUrl + projectName + id + userName + teamName + eMail + usesName);
        } catch (Exception e) {
        	if(Logging.WARNING) Log.w(Logging.TAG, "AttachProjectWorkingActivity error while parsing extras", e);
        	finish(); // no point to continue without data
        }
        
        // get coniguration
        timeInterval = getResources().getInteger(R.integer.attach_step_interval_ms);
        
        // set layout
		setContentView(R.layout.attach_project_working_layout);
		anchor = (ViewGroup) findViewById(R.id.anchor);
        
        // set title bar
        getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.title_bar);
    }
    
	@Override
	protected void onDestroy() {
    	if(Logging.VERBOSE) Log.v(Logging.TAG, "AttachProjectWorkingActivity onDestroy");
	    doUnbindService();
	    super.onDestroy();
	}

	private void doBindService() {
	    // Establish a connection with the service, onServiceConnected gets called when
		bindService(new Intent(this, Monitor.class), mConnection, Service.BIND_AUTO_CREATE);
	}

	private void doUnbindService() {
	    if (mIsBound) {
	        // Detach existing connection.
	        unbindService(mConnection);
	        mIsBound = false;
	    }
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
	
	private String mapErrorNumToString(int code) {
		if(Logging.DEBUG) Log.d(Logging.TAG,"mapErrorNumToString for error: " + code);
		int stringResource;
		switch (code) {
		case BOINCErrors.ERR_DB_NOT_FOUND:
			stringResource = R.string.attachproject_error_wrong_name;
			break;
		case BOINCErrors.ERR_GETHOSTBYNAME:
			stringResource = R.string.attachproject_error_no_internet;
			break;
		case BOINCErrors.ERR_NONUNIQUE_EMAIL: // treat the same as -137, ERR_DB_NOT_UNIQUE
			// no break!!
		case BOINCErrors.ERR_DB_NOT_UNIQUE:
			stringResource =  R.string.attachproject_error_email_in_use;
			break;
		case BOINCErrors.ERR_PROJECT_DOWN:
			stringResource = R.string.attachproject_error_project_down;
			break;
		case BOINCErrors.ERR_BAD_EMAIL_ADDR:
			stringResource =  R.string.attachproject_error_email_bad_syntax;
			break;
		case BOINCErrors.ERR_BAD_PASSWD:
			stringResource = R.string.attachproject_error_bad_pwd;
			break;
		case BOINCErrors.ERR_BAD_USER_NAME:
			stringResource = R.string.attachproject_error_bad_username;
			break;
		case BOINCErrors.ERR_ACCT_CREATION_DISABLED:
			stringResource = R.string.attachproject_error_creation_disabled;
			break;
		default:
			stringResource = R.string.attachproject_error_unknown;
			break;
		}
		return getString(stringResource);
	}
	
	// appends new element to layout
	private void appendElementToLayout(Update update) {
		LinearLayout newElement;
		String text;
		
		// setup view from layout
		if(update.finalResult) { // final result is either success or failure
			// remove working layout
			View previousElement = views.get(views.size()-1);
			anchor.removeView(previousElement);
			//inflate new final layout
			if(update.success) {
				newElement = (LinearLayout)getLayoutInflater().inflate(R.layout.attach_project_success_layout, anchor, false);
				text = getString(update.task) + getString(R.string.attachproject_working_finished);
			} else {
				newElement = (LinearLayout)getLayoutInflater().inflate(R.layout.attach_project_failed_layout, anchor, false);
				String desc = "";
				if(!update.result.isEmpty()) desc = getString(R.string.attachproject_working_description) + " " + update.result;
				if(update.result.equals(getString(R.string.attachproject_error_unknown))) desc = getString(R.string.attachproject_working_description) + " " + update.errorCode + " " + update.result;
				text = getString(update.task) + desc;
			}
		} else {
			//inflate new working layout
			newElement = (LinearLayout)getLayoutInflater().inflate(R.layout.attach_project_ongoing_layout, anchor, false);
			text = getString(update.task) + getString(R.string.attachproject_working_ongoing);
		}
		
		// set header text
		TextView header = (TextView) newElement.findViewById(R.id.header);
		header.setText(text);
		
		// add view
		anchor.addView(newElement);
		views.add(newElement);
	}
	
	public void finishButtonClicked(View view) {
		Intent intent = new Intent(this, BOINCActivity.class);
		intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP); //clear_top closes AttachProjectListActivity!
		startActivity(intent);
	}
	
	public void backButtonClicked(View view) {
		finish();
	}
	
	private class Update {
		public Boolean finalResult;
		public Boolean success;
		public int task;
		public String result = "";
		public int errorCode = -1;
		
		public Update(Boolean finalResult, Boolean success, int task, String result, int errorCode){
			this.finalResult = finalResult;
			this.success = success;
			this.task = task;
			this.result = result;
			this.errorCode = errorCode;
		}
	}
	
	private final class ProjectAccountAsync extends AsyncTask<Void, Update, Boolean> {
		
		private Boolean registration;
		private String url;
		private String id; // used for login can be either email or user, depending on usesName
		private String email;
		private String userName;
		private String teamName;
		private String pwd;
		private Boolean usesName;
		private String projectName;
		
		public ProjectAccountAsync(Boolean registration, String url, String id, String email, String userName, String teamName, String pwd, Boolean usesName, String projectName) {
			this.registration = registration;
			this.url = url;
			this.id = id; // used for login
			this.email = email;
			this.userName = userName;
			this.teamName = teamName;
			this.pwd = pwd;
			this.usesName = usesName;
			this.projectName = projectName;
		}
		
		@Override
		protected Boolean doInBackground(Void... params) {
			if(Logging.DEBUG) Log.d(Logging.TAG,"ProjectAccountAsync doInBackground");
			
			//check device online
			publishProgress(new Update(false, false, R.string.attachproject_working_connect,"",0));
			try {Thread.sleep(timeInterval);} catch (Exception e){}
			if(!checkDeviceOnline()) {
				publishProgress(new Update(true, false, R.string.attachproject_working_connect, getString(R.string.attachproject_error_no_internet),-1));
				return false;
			}
			publishProgress(new Update(true, true, R.string.attachproject_working_connect,"",0));
			
			// get authenticator
			AccountOut account = null;
			Integer attemptCounter = 0;
			Integer maxAttempts = 0;
			Boolean success = false;
			int err = -1;
			if(registration) {
				// register account
				publishProgress(new Update(false, false, R.string.attachproject_working_register,"",0));
				maxAttempts = getResources().getInteger(R.integer.attach_creation_retries);
				if(Logging.DEBUG) Log.d(Logging.TAG,"registration with: " + url + email + userName + teamName + maxAttempts);
				// retry a defined number of times, if non deterministic failure occurs.
				// makes login more robust on bad network connections
				while(!success && attemptCounter < maxAttempts) {
					account = monitor.createAccount(url, email, userName, pwd, teamName);
					
					if(account == null || account.error_num != BOINCErrors.ERR_OK) {
						// failed
						if(account != null) err = account.error_num;
						if(Logging.DEBUG) Log.d(Logging.TAG,"registration failed, error code: " + err);
						if(err == -1 || err == BOINCErrors.ERR_GETHOSTBYNAME){
							// worth a retry
							attemptCounter++;
						} else {
							// not worth a retry, return
							publishProgress(new Update(true, false, R.string.attachproject_working_register, mapErrorNumToString(err),err));
							return false;
						}
					} else {
						// successful
						try {Thread.sleep(timeInterval);} catch (Exception e){}
						publishProgress(new Update(true, true, R.string.attachproject_working_register,"",0));
						success = true;
					}
				}
				// reached end of loop, check if successful
				if(!success) {
					publishProgress(new Update(true, false, R.string.attachproject_working_register, mapErrorNumToString(err),err));
					return false;
				}
			} else {
				// lookup authenticator
				publishProgress(new Update(false, false, R.string.attachproject_working_verify,"",0));
				maxAttempts = getResources().getInteger(R.integer.attach_login_retries);
				if(Logging.DEBUG) Log.d(Logging.TAG,"loging with: " + url + id + usesName + maxAttempts);
				// retry a defined number of times, if non deterministic failure occurs.
				// makes login more robust on bad network connections
				while(!success && attemptCounter < maxAttempts) {
					account = monitor.lookupCredentials(url, id, pwd, usesName);
					
					if(account == null || account.error_num != BOINCErrors.ERR_OK) {
						// failed
						if(account != null) err = account.error_num;
						if(Logging.DEBUG) Log.d(Logging.TAG,"registration failed, error code: " + err);
						if(err == -1 || err == BOINCErrors.ERR_GETHOSTBYNAME){
							// worth a retry
							attemptCounter++;
						} else {
							// not worth a retry, return
							publishProgress(new Update(true, false, R.string.attachproject_working_verify, mapErrorNumToString(err), err));
							return false;
						}
					} else {
						// successful
						try {Thread.sleep(timeInterval);} catch (Exception e){}
						publishProgress(new Update(true, true, R.string.attachproject_working_verify,"",0));
						success = true;
					}
				}
				// reached end of loop, check if successful
				if(!success) {
					publishProgress(new Update(true, false, R.string.attachproject_working_verify, mapErrorNumToString(err), err));
					return false;
				}
			}
			
			// attach project
			attemptCounter = 0;
			success = false;
			maxAttempts = getResources().getInteger(R.integer.attach_attach_retries);
			publishProgress(new Update(false, false, R.string.attachproject_working_login,"",0));
			while(!success && attemptCounter < maxAttempts) {
				Boolean attach = monitor.attachProject(url, projectName, account.authenticator);
				if(attach) {
					// successful
					success = true;
					try {Thread.sleep(timeInterval);} catch (Exception e){}
					publishProgress(new Update(true, true, R.string.attachproject_working_login,"",0));
				} else {
					// failed
					attemptCounter++;
				}
			}
			if(!success) {
				// still failed
				publishProgress(new Update(true, false, R.string.attachproject_working_login,"",0));
				return false;
			}
			
			return true;
		}		
		
		@Override
		protected void onProgressUpdate(Update... values) {
			appendElementToLayout(values[0]);
			super.onProgressUpdate(values);
		}
		
		protected void onPostExecute(Boolean result) {
			if(result) {
				// show finish button
				Button finishButton = (Button) findViewById(R.id.finishButton);
				finishButton.setVisibility(View.VISIBLE);
			} else {
				// show back button
				Button backButton = (Button) findViewById(R.id.backButton);
				backButton.setVisibility(View.VISIBLE);
			}
		}
	}
}
