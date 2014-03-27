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
import android.app.Activity;
import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Paint;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class CredentialInputActivity extends Activity{
	
	private EditText emailET;
	private EditText nameET;
	private EditText pwdET;
	
	private ProjectAttachService attachService = null;
	private boolean asIsBound = false;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState); 
        if(Logging.DEBUG) Log.d(Logging.TAG, "CredentialInputActivity onCreate"); 
        doBindService();
        setContentView(R.layout.attach_project_credential_input_layout);  
        emailET = (EditText) findViewById(R.id.email_input);
        nameET = (EditText) findViewById(R.id.name_input);
        pwdET = (EditText) findViewById(R.id.pwd_input);
        
        ((TextView) findViewById(R.id.individual_login_button)).setPaintFlags(Paint.UNDERLINE_TEXT_FLAG);
    }
	
	@Override
	protected void onDestroy() {
		doUnbindService();
		super.onDestroy();
	}

	// triggered by continue button
	public void continueClicked(View v) {
        if(Logging.DEBUG) Log.d(Logging.TAG, "CredentialInputActivity.continueClicked.");
        
        // verfiy input, return if failed.
		if(!verifyInput(emailET.getText().toString(), nameET.getText().toString(), pwdET.getText().toString(), null)) return;
		
		// set credentials in service
		if(asIsBound) attachService.setCredentials(emailET.getText().toString(), nameET.getText().toString(), pwdET.getText().toString());
		else {
			if(Logging.ERROR) Log.e(Logging.TAG, "CredentialInputActivity.continueClicked: service not bound.");
			return;
		}

        if(Logging.DEBUG) Log.d(Logging.TAG, "CredentialInputActivity.continueClicked: starting BatchProcessingActivity...");
		startActivity(new Intent(this, BatchProcessingActivity.class));
	}	
	
	// triggered by individual button
	public void individualClicked(View v) {
        if(Logging.DEBUG) Log.d(Logging.TAG, "CredentialInputActivity.individualClicked.");

		// set credentials in service, in case user typed before deciding btwn batch and individual attach
		if(asIsBound) attachService.setCredentials(emailET.getText().toString(), nameET.getText().toString(), pwdET.getText().toString());
		
		//startActivity(new Intent(this, IndividualAttachActivity.class));
		Intent intent = new Intent(this, BatchConflictListActivity.class);
		intent.putExtra("conflicts", false);
		startActivity(new Intent(this, BatchConflictListActivity.class));
	}
	
	private Boolean verifyInput(String email, String user, String pwd, String pwdConfirm) {
		int stringResource = 0;
		
		// check input
		if(email.length() == 0) {
			stringResource = R.string.attachproject_error_no_email;
		}
		else if(user.length() == 0) {
			stringResource = R.string.attachproject_error_no_name;
		}
		else if(pwd != null && pwd.length() == 0) {
			stringResource = R.string.attachproject_error_no_pwd;
		}
		else if(pwd != null && pwd.length() < 6) { // appropriate for min pwd length?!
			stringResource = R.string.attachproject_error_short_pwd;
		}
		else if(pwdConfirm != null && pwdConfirm.length() == 0) {
			stringResource = R.string.attachproject_error_pwd_no_retype;
		}
		else if(pwd != null && pwdConfirm != null && !pwd.equals(pwdConfirm)) {
			stringResource = R.string.attachproject_error_pwd_no_match;
		}
		
		if(stringResource != 0) {
			Toast toast = Toast.makeText(getApplicationContext(), stringResource, Toast.LENGTH_SHORT);
			toast.show();
			return false;
		} else return true;
	}
	
	private ServiceConnection mASConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        // This is called when the connection with the service has been established, getService returns 
	    	// the Monitor object that is needed to call functions.
	        attachService = ((ProjectAttachService.LocalBinder)service).getService();
		    asIsBound = true;
		    
		    ArrayList<String> values = attachService.getUserDefaultValues();
	        emailET.setText(values.get(0));
	        nameET.setText(values.get(1));
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
}
