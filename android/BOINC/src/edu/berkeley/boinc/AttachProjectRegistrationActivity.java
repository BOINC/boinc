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

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.widget.EditText;
import android.widget.TextView;

public class AttachProjectRegistrationActivity extends Activity{
	
	private String projectUrl;
	private String projectName;
	private Integer minPwdLength;
	private Boolean usesName;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState);  
        if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectRegistrationActivity onCreate"); 
        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);

    	//parse master url from intent extras
        try {
        	projectUrl = getIntent().getCharSequenceExtra("projectUrl").toString();
        	projectName = getIntent().getCharSequenceExtra("projectName").toString();
        	minPwdLength = getIntent().getIntExtra("minPwdLength", 0);
        	usesName = getIntent().getBooleanExtra("usesName", false);
        	if(Logging.DEBUG) Log.d(Logging.TAG,"intent extras: " + projectUrl + projectName + minPwdLength);
        } catch (Exception e) {
        	if(Logging.WARNING) Log.w(Logging.TAG, "error while parsing url", e);
        	finish(); // no point to continue without url
        }
        
		// setup layout
		setContentView(R.layout.attach_project_registration_layout);
        
        // set title bar
        getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.title_bar);
		
		TextView headerName = (TextView) findViewById(R.id.registration_header);
		headerName.setText(getString(R.string.attachproject_registration_header) + " " + projectName);
		TextView urlTv = (TextView) findViewById(R.id.url);
		urlTv.setText(projectUrl);
    }
    
	@Override
	protected void onDestroy() {
    	if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectRegistrationActivity onDestroy");
	    super.onDestroy();
	}
	
	// onclick of button
	public void register (View view) {		
		// get user input
		EditText emailInput = (EditText) findViewById(R.id.email_input);
		EditText userInput = (EditText) findViewById(R.id.username_input);
		EditText teamInput = (EditText) findViewById(R.id.teamname_input);
		EditText pwdInput = (EditText) findViewById(R.id.pwd_input);
		EditText pwdConfirmInput = (EditText) findViewById(R.id.pwd_confirm_input);
		String email = emailInput.getText().toString();
		String user = userInput.getText().toString();
		String team = teamInput.getText().toString();
		String pwd = pwdInput.getText().toString();
		String pwdConfirm = pwdConfirmInput.getText().toString();
		
		// verify and start AttachProjectWorkingActivity
		if(verifyInput(email, user, team, pwd, pwdConfirm, usesName)){
			Intent intent = new Intent(this, AttachProjectWorkingActivity.class);
			intent.putExtra("registration", true);
			intent.putExtra("usesName", false);
			intent.putExtra("projectUrl", projectUrl);
			intent.putExtra("projectName", projectName);
			intent.putExtra("userName", user);
			intent.putExtra("teamName", team);
			intent.putExtra("eMail", email);
			intent.putExtra("pwd", pwd);
			startActivity(intent);
		}
	}
	
	private Boolean verifyInput(String email, String user, String team, String pwd, String pwdConfirm, Boolean usesName) {
		int stringResource = R.string.attachproject_error_unknown;
		Boolean success = true;
		
		// check input
		if(email.length() == 0) {
			stringResource = R.string.attachproject_error_no_email;
			success = false;
		}
		else if(usesName && user.length() == 0) {
			stringResource = R.string.attachproject_error_no_name;
			success = false;
		}
		else if(pwd.length() == 0) {
			stringResource = R.string.attachproject_error_no_pwd;
			success = false;
		}
		else if(pwd.length() < minPwdLength) {
			stringResource = R.string.attachproject_error_short_pwd;
			success = false;
		}
		else if(!pwd.equals(pwdConfirm)) {
			stringResource = R.string.attachproject_error_pwd_no_match;
			success = false;
		}
		
		// show warning
		TextView warning = (TextView) findViewById(R.id.warning);
		if(!success) {
			warning.setText(stringResource);
			warning.setVisibility(View.VISIBLE);
		} else {
			// making sure previous warnings are gone
			warning.setVisibility(View.GONE);
		}
		
		return success;
	}
}
