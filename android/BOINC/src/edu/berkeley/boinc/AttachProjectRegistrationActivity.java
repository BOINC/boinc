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

import edu.berkeley.boinc.client.Monitor;
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
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.utils.BOINCErrors;

public class AttachProjectRegistrationActivity extends Activity{
	
	private final String TAG = "BOINC AttachProjectRegistrationActivity"; 
	
	private Monitor monitor;
	private Boolean mIsBound;
	
	private String projectUrl;
	private String projectName;
	private Integer minPwdLength;
	
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        // This is called when the connection with the service has been established, getService returns the Monitor object that is needed to call functions.
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
	    }

	    public void onServiceDisconnected(ComponentName className) { // This should not happen
	        monitor = null;
		    mIsBound = false;
	    }
	};
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState);  
        Log.d(TAG, "onCreate"); 
        
        // bind monitor service
        doBindService();

    	//parse master url from intent extras
        try {
        	projectUrl = getIntent().getCharSequenceExtra("projectUrl").toString();
        	projectName = getIntent().getCharSequenceExtra("projectName").toString();
        	minPwdLength = getIntent().getIntExtra("minPwdLength", 0);
        	Log.d(TAG,"intent extras: " + projectUrl + projectName + minPwdLength);
        } catch (Exception e) {
        	Log.w(TAG, "error while parsing url", e);
        	finish(true); // no point to continue without url
        }
        
        populateLayout();
    }
    
	@Override
	protected void onDestroy() {
    	Log.d(TAG, "onDestroy");
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
	
	public void finish(Boolean forced){
		if(forced) {
			Toast toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_error_whoops, Toast.LENGTH_LONG);
			toast.show();
		}
		super.finish();
	}
	
	// gets called by GetProjectConfig when ProjectConfig is available
	private void populateLayout() {
		Log.d(TAG, "populateLayout");
		
		setContentView(R.layout.attach_project_registration_layout);
		
		TextView headerName = (TextView) findViewById(R.id.registration_header);
		headerName.setText(getString(R.string.attachproject_registration_header) + " " + projectName);
		TextView urlTv = (TextView) findViewById(R.id.url);
		urlTv.setText(projectUrl);
	}
	
	public void register (View view) {
		new ProjectRegisterAsync().execute();
	}
	
	private Boolean verifyInput(String email, String user, String team, String pwd, String pwdConfirm) {
		if(email.length() == 0) {
			Toast toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_error_no_email, Toast.LENGTH_SHORT);
			toast.show();
			return false;
		}
		if(pwd.length() == 0) {
			Toast toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_error_no_pwd, Toast.LENGTH_SHORT);
			toast.show();
			return false;
		}
		if(pwd.length() < minPwdLength) {
			Toast toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_error_short_pwd, Toast.LENGTH_SHORT);
			toast.show();
			return false;
		}
		if(!pwd.equals(pwdConfirm)) {
			Toast toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_error_pwd_no_match, Toast.LENGTH_SHORT);
			toast.show();
			return false;
		}
		return true;
	}
	
	private void showResultToast(Integer code) {
		Log.d(TAG,"showResultToast for error: " + code);
		Toast toast;
		if(code == null) {
			toast = Toast.makeText(getApplicationContext(), R.string.attachproject_login_toast_error_unknown, Toast.LENGTH_LONG);
		} else {
			switch (code) {
			case BOINCErrors.ERR_OK:
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_login_successful, Toast.LENGTH_LONG);
				break;
			case BOINCErrors.ERR_NONUNIQUE_EMAIL: // treat the same as -137, ERR_DB_NOT_UNIQUE
				// no break!!
			case BOINCErrors.ERR_DB_NOT_UNIQUE:
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_error_email_in_use, Toast.LENGTH_LONG);
				break;
			case BOINCErrors.ERR_PROJECT_DOWN:
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_error_project_down, Toast.LENGTH_LONG);
				break;
			case BOINCErrors.ERR_BAD_EMAIL_ADDR:
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_error_email_bad_syntax, Toast.LENGTH_LONG);
				break;
			case BOINCErrors.ERR_BAD_PASSWD:
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_error_bad_pwd, Toast.LENGTH_LONG);
				break;
			case BOINCErrors.ERR_BAD_USER_NAME:
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_error_bad_username, Toast.LENGTH_LONG);
				break;
			case BOINCErrors.ERR_GETHOSTBYNAME:
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_login_toast_error_no_internet, Toast.LENGTH_LONG);
				break;
			case BOINCErrors.ERR_ACCT_CREATION_DISABLED:
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_error_creation_disabled, Toast.LENGTH_LONG);
				break;
			default:
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_login_toast_error_unknown, Toast.LENGTH_LONG);
				break;
			}
		}
		toast.show();
	}
	
	private void changeLayoutLoggingIn(Boolean loggingIn) {
		Log.d(TAG,"changeLayoutLoggingIn: " + loggingIn);
		LinearLayout inputWrapper = (LinearLayout) findViewById(R.id.input_wrapper);
		ProgressBar registrationLoading = (ProgressBar) findViewById(R.id.registration_pending);
		
		if(loggingIn) {
			inputWrapper.setVisibility(View.GONE);
			registrationLoading.setVisibility(View.VISIBLE);
		} else {
			registrationLoading.setVisibility(View.GONE);
			inputWrapper.setVisibility(View.VISIBLE);
		}
	}
	
	private void goToMainActivity() {
		Intent intent = new Intent(this, BOINCActivity.class);
		intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP); //clear_top closes AttachProjectListActivity!
		startActivity(intent);
	}
	
	private final class ProjectRegisterAsync extends AsyncTask<Void, Void, Integer> {

		//private final String TAG = "ProjectRegisterAsync";
		
		private String email;
		private String user;
		private String team;
		private String pwd;
		private String pwdConfirm;
		
		@Override 
		protected void onPreExecute() {
			changeLayoutLoggingIn(true);
			
			// read input data
			EditText emailInput = (EditText) findViewById(R.id.email_input);
			EditText userInput = (EditText) findViewById(R.id.username_input);
			EditText teamInput = (EditText) findViewById(R.id.teamname_input);
			EditText pwdInput = (EditText) findViewById(R.id.pwd_input);
			EditText pwdConfirmInput = (EditText) findViewById(R.id.pwd_confirm_input);
			email = emailInput.getText().toString();
			user = userInput.getText().toString();
			team = teamInput.getText().toString();
			pwd = pwdInput.getText().toString();
			pwdConfirm = pwdConfirmInput.getText().toString();

			if(!verifyInput(email, user, team, pwd, pwdConfirm)) {
				changeLayoutLoggingIn(false);
				cancel(true); // cancel asynctask if validation fails
			}
		}
		
		@Override
		protected Integer doInBackground(Void... params) {
			
			AccountOut account = monitor.createAccount(projectUrl, email, user, pwd, team);
			
			if(account == null) {
				Log.d(TAG, "createAccount returned null");
				return null;
			}
			
			if(account.error_num == BOINCErrors.ERR_OK) { //only continue if creation succeeded
				publishProgress();
				Boolean attach = monitor.attachProject(projectUrl, email, account.authenticator);
				if(attach) {
					return BOINCErrors.ERR_OK;
				} else {
					Log.d(TAG, "attachProject returned false");
					return null;
				}
			} else { // error code
				return account.error_num;
			}
		}
		
		@SuppressWarnings("unused") // used by calling publishProgress()
		protected void onProgressUpdate() {
			//only show positive progress, others are shown in onPostExecute
			Toast toast = Toast.makeText(getApplicationContext(), R.string.attachproject_registration_toast_creation_successful, Toast.LENGTH_SHORT);
			toast.show();
		}
		
		@Override
		protected void onPostExecute(Integer errorCode) {
			showResultToast(errorCode);
			if((errorCode != null) && (errorCode == BOINCErrors.ERR_OK)) { //successful
				monitor.forceRefresh(); // force refresh, so "no project banner" disappears
				goToMainActivity();
			}
			changeLayoutLoggingIn(false);
		}
	}
}
