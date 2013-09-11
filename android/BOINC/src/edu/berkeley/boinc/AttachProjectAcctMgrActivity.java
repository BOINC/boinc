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

public class AttachProjectAcctMgrActivity extends Activity{
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState);  
        if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectAcctMgrActivity onCreate"); 
        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
        
		// setup layout
		setContentView(R.layout.attach_project_acctmgr_layout);
        
        // set title bar
        getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.title_bar);
    }
    
	@Override
	protected void onDestroy() {
    	if(Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectAcctMgrActivity onDestroy");
	    super.onDestroy();
	}
	
	// onclick of button
	public void addAcctMgrClick(View view) {		
		// get user input
		EditText urlInput = (EditText) findViewById(R.id.url_input);
		EditText nameInput = (EditText) findViewById(R.id.name_input);
		EditText pwdInput = (EditText) findViewById(R.id.pwd_input);
		EditText pwdConfirmInput = (EditText) findViewById(R.id.pwd_confirm_input);
		String url = urlInput.getText().toString();
		String name = nameInput.getText().toString();
		String pwd = pwdInput.getText().toString();
		String pwdConfirm = pwdConfirmInput.getText().toString();
		
		// verify and start AttachProjectWorkingActivity
		if(verifyInput(url, name, pwd, pwdConfirm)){
			Intent intent = new Intent(this, AttachProjectWorkingActivity.class);
			intent.putExtra("action", AttachProjectWorkingActivity.ACTION_ACCTMGR);
			intent.putExtra("projectUrl", url);
			intent.putExtra("userName", name);
			intent.putExtra("pwd", pwd);
			startActivity(intent);
		}
	}
	
	private Boolean verifyInput(String url, String name, String pwd, String pwdConfirm) {
		int stringResource = R.string.attachproject_error_unknown;
		Boolean success = true;
		
		// check input
		if(url.length() == 0) {
			stringResource = R.string.attachproject_error_no_url;
			success = false;
		}
		else if(name.length() == 0) {
			stringResource = R.string.attachproject_error_no_name;
			success = false;
		}
		else if(pwd.length() == 0) {
			stringResource = R.string.attachproject_error_no_pwd;
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
