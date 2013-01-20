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
import android.app.Activity;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

public class LoginActivity extends Activity {
	
	private final String TAG = "LoginActivity"; 
	
	private Monitor monitor;
	public static ClientStatus clientStatus;
	
	private Boolean mIsBound;
	
	public final static int BROADCAST_TYPE_LOGIN = 1;
	public final static int BROADCAST_TYPE_REGISTRATION = 2;
	public final static int RESULT_OK = 0;
	public final static int RESULT_PWD_INCORRECT = -206;
	public final static int RESULT_EMAIL_INCORRECT = -136;
	public final static int RESULT_NO_CONNECTION = -113;

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
	
	private BroadcastReceiver mLoginResultsRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context,Intent intent) {
			Log.d(TAG+"-loginresults","received");
			Integer type = intent.getIntExtra("type", 0);
			Integer result = intent.getIntExtra("result", -1);
			String message = intent.getStringExtra("message");
			parseResults(type,result,message);
		}
	};
	private IntentFilter iflr = new IntentFilter("edu.berkeley.boinc.loginresults");
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState);  
        setContentView(R.layout.login_layout);  
         
        Log.d(TAG, "onCreate"); 
        
        //bind monitor service
        doBindService();

        ((EditText) findViewById(R.id.urlIn)).setText(R.string.project_url); //TODO remove from final build
    }
    
	@Override
	protected void onDestroy() {
    	Log.d(TAG, "onDestroy");
	    doUnbindService();
	    super.onDestroy();
	}

	@Override
	protected void onResume() { // gets called by system every time activity comes to front. after onCreate upon first creation
    	Log.d(TAG, "onResume");
	    super.onResume();
	    registerReceiver(mLoginResultsRec,iflr);
	}

	@Override
	protected void onPause() { // gets called by system every time activity loses focus.
    	Log.d(TAG, "onPause");
	    super.onPause();
	    unregisterReceiver(mLoginResultsRec);
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
    
	public void loginButtonClicked (View view) {
		Log.d(TAG,"loginButtonClicked");
		
		if(!mIsBound) { //monitor not bound, cant call commands, return...
			Log.d(TAG,"monitor not bound, return...");
			return;
		}
		
		//read input data
		EditText urlET = (EditText) findViewById(R.id.urlIn);
		EditText emailET = (EditText) findViewById(R.id.emailIn);
		EditText pwdET = (EditText) findViewById(R.id.pwdIn);
		String url = urlET.getText().toString();
		String email = emailET.getText().toString();
		String pwd = pwdET.getText().toString();
		Button button = (Button) findViewById(R.id.loginButton);
		Object buttonTag = button.getTag();
		Log.d(TAG,"Input data: " + url + " - " + email + " - " + pwd + " - " + buttonTag);
		
		//preventing 0 input
		if(url.length()==0) {
			Toast toast = Toast.makeText(this, R.string.login_no_url, Toast.LENGTH_SHORT);
			toast.show();
			return;
		}
		if(email.length()==0) {
			Toast toast = Toast.makeText(this, R.string.login_no_email, Toast.LENGTH_SHORT);
			toast.show();
			return;
		}
		if(pwd.length()==0) {
			Toast toast = Toast.makeText(this, R.string.login_no_pwd, Toast.LENGTH_SHORT);
			toast.show();
			return;
		}
		
		if(buttonTag.equals("login")) { //login
			button.setVisibility(View.GONE);
			((ProgressBar) findViewById(R.id.pendingResultButtonReplacement)).setVisibility(View.VISIBLE);
			monitor.attachProjectAsync(url, "", email, pwd);
		} else { //register
			//read additional data
			EditText userNameET = (EditText) findViewById(R.id.userIn);
			EditText teamNameET = (EditText) findViewById(R.id.teamIn);
			EditText pwdConfirmET = (EditText) findViewById(R.id.pwdConfirmIn);
			String pwdConfirm = pwdConfirmET.getText().toString();
			String userName = userNameET.getText().toString();
			String teamName = teamNameET.getText().toString();
			Log.d(TAG,"Additional data: " + userName + " - " + teamName + " - " + pwdConfirm);
			
			if(!pwd.equals(pwdConfirm)) { //check whether passwords are equal
				Toast toast = Toast.makeText(this, R.string.login_pwd_nomatch, Toast.LENGTH_SHORT);
				toast.show();
				return;
			}

			button.setVisibility(View.GONE);
			((ProgressBar) findViewById(R.id.pendingResultButtonReplacement)).setVisibility(View.VISIBLE);
			monitor.createAccountAsync(url, email, userName, pwd, teamName);
		}
	}

	public void changeLayout(View view) { 
		Log.d(TAG,"changeLayout");
		TextView header = (TextView) findViewById(R.id.login_textView_header_long);
		LinearLayout userName = (LinearLayout) findViewById(R.id.login_ll_username);
		LinearLayout teamName = (LinearLayout) findViewById(R.id.login_ll_teamname);
		LinearLayout pwdConfirm = (LinearLayout) findViewById(R.id.login_ll_pwdconfirm);
		TextView registerHint = (TextView) findViewById(R.id.login_textView_register_hint);
		TextView changeMode = (TextView) findViewById(R.id.login_textView_changemode);
		Button button = (Button) findViewById(R.id.loginButton);
		
		Object buttonTag = button.getTag();
		
		if(buttonTag.equals("login")) { //layout currently for login, change to register
			header.setText(R.string.login_header_long_register);
			userName.setVisibility(View.VISIBLE);
			teamName.setVisibility(View.VISIBLE);
			pwdConfirm.setVisibility(View.VISIBLE);
			registerHint.setVisibility(View.VISIBLE);
			changeMode.setText(R.string.login_login_instead);
			button.setText(R.string.login_button_register);
			button.setTag("register");
		} else { //layout currently for register, change to login
			header.setText(R.string.login_header_long_login);
			userName.setVisibility(View.GONE);
			teamName.setVisibility(View.GONE);
			pwdConfirm.setVisibility(View.GONE);
			registerHint.setVisibility(View.GONE);
			changeMode.setText(R.string.login_register_instead);
			button.setText(R.string.login_button_login);
			button.setTag("login");
		}
	}
	
	private void parseResults(Integer type, Integer result, String message) {

		((Button) findViewById(R.id.loginButton)).setVisibility(View.VISIBLE);
		((ProgressBar) findViewById(R.id.pendingResultButtonReplacement)).setVisibility(View.GONE);
		String toastMessage = "";
		
		switch (result) {
		case LoginActivity.RESULT_OK:
			toastMessage = getResources().getString(R.string.login_toast_ok);
			break;
		case LoginActivity.RESULT_EMAIL_INCORRECT:
			toastMessage = getResources().getString(R.string.login_toast_error_wrong_name);
			break;
		case LoginActivity.RESULT_PWD_INCORRECT:
			toastMessage = getResources().getString(R.string.login_toast_error_wrong_pwd);
			break;
		case LoginActivity.RESULT_NO_CONNECTION:
			toastMessage = getResources().getString(R.string.login_toast_error_no_internet);
			break;
		default:
			toastMessage = getResources().getString(R.string.login_toast_error_unknown);
			break;
		}
		
		Toast toast = Toast.makeText(this, toastMessage, Toast.LENGTH_SHORT);
		toast.show();
		
		if(result == LoginActivity.RESULT_OK) { //successful
			monitor.forceRefresh();
			finish(); //close activity
		}
	}
}
