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

import java.net.URL;
import edu.berkeley.boinc.client.Monitor;
import android.app.Activity;
import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.text.InputType;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.rpc.PlatformInfo;
import edu.berkeley.boinc.rpc.ProjectConfig;
import edu.berkeley.boinc.rpc.ProjectInfo;
import edu.berkeley.boinc.utils.BOINCErrors;

public class AttachProjectLoginActivity extends Activity{
	
	private final String TAG = "BOINC AttachProjectLoginActivity"; 
	
	private Monitor monitor;
	private Boolean mIsBound;
	
	private String url;
	private ProjectInfo projectInfo;
	private Boolean projectInfoPresent = false; // complete ProjectInfo available, if selection from list
	private ProjectConfig projectConfig;
	private Bitmap projectLogo;
	
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        // This is called when the connection with the service has been established, getService returns the Monitor object that is needed to call functions.
	        monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
		    
		    (new GetProjectConfig()).execute((Void[]) null);
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

    	//parse master url from intent extras
        Boolean urlPresent = false;
        try {
        	url = getIntent().getCharSequenceExtra("url").toString();
        	Log.d(TAG,"url: " + url);
        	if(url != null) urlPresent = true;
        } catch (Exception e) {}
        
        //parse  project info from intent extras
        try {
        	projectInfo = (ProjectInfo) getIntent().getSerializableExtra("projectInfo");
        	Log.d(TAG,"projectInfo: " + projectInfo);
        	if(projectInfo != null) {
        		projectInfoPresent = true;
        		url = projectInfo.url; // set url field to information of projectInfo
        	}
        } catch (Exception e) {Log.d(TAG,"no project info...");}
        
        if(!projectInfoPresent) { // url can not be taken of ProjectInfo
        	// format user input on URL right to avoid exceptions
        	if (!url.startsWith("http://") && !url.startsWith("https://")) url = "http://" + url; // add http:// in case user leaves it out
        	if (!url.endsWith("/")) url = url + "/"; // add trailing slash
        }
        
        if(!urlPresent && !projectInfoPresent) {
        	// neither url (manual input) nor project info (list selection) is present
        	Log.d(TAG,"neither url nor projectInfo available! finish activity...");
        	finish(R.string.attachproject_login_error_toast);
        }
        
        // setup layout
        setContentView(R.layout.attach_project_login_layout_loading);
        TextView urlView = (TextView) findViewById(R.id.login_loading_url);
      	urlView.setText(url);
        
        // bind monitor service
        doBindService();
    }
    
	@Override
	protected void onDestroy() {
    	Log.d(TAG, "onDestroy");
	    doUnbindService();
	    super.onDestroy();
	}
	
	public void finish(Integer toastStringId){
		Toast toast = Toast.makeText(getApplicationContext(), toastStringId, Toast.LENGTH_LONG);
		toast.show();
		super.finish();
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
	
	// gets called by GetProjectConfig when ProjectConfig is available
	private void populateLayout() {
		Log.d(TAG, "populateLayout");
		
		setContentView(R.layout.attach_project_login_layout);
		
		// set name
		TextView name = (TextView) findViewById(R.id.project_name);
		if(projectLogo != null) {
			name.setVisibility(View.GONE); // disable name if logo present
		} else {
			name.setText(projectConfig.name);
		}
		
		// set website
		TextView website = (TextView) findViewById(R.id.project_url);
		website.setText(projectConfig.masterUrl);
		website.setTag(projectConfig.masterUrl); // set tag to use in onClick
		
		// set android support
		TextView platform = (TextView) findViewById(R.id.project_platform);
		if(platformSupported()) {
			platform.setText(R.string.attachproject_login_platform_supported);
		} else {
			platform.setText(R.string.attachproject_login_platform_not_supported);
			platform.setTextColor(getResources().getColor(R.color.warning));
		}
		
		// set ProjectInfo fields, if ProjectInfo available (after list selection)
		if(projectInfoPresent) {
			// set layout wrapper visible
			LinearLayout projectInfoWrapper = (LinearLayout) findViewById(R.id.project_info);
			projectInfoWrapper.setVisibility(View.VISIBLE);
			
			// set project logo
			ImageView logo = (ImageView) findViewById(R.id.logo);
			if(projectLogo != null) {
				logo.setImageBitmap(projectLogo);
			} else {
				logo.setVisibility(View.GONE);
			}
			
			// set general area
			if(projectInfo.generalArea != null) {
				TextView generalArea = (TextView) findViewById(R.id.general_area);
				generalArea.setText(projectInfo.generalArea);
			} else {
				LinearLayout wrapper = (LinearLayout) findViewById(R.id.general_area_wrapper);
				wrapper.setVisibility(View.GONE);
			}
			
			// set specific area
			if(projectInfo.specificArea != null) {
				TextView specificArea = (TextView) findViewById(R.id.specific_area);
				specificArea.setText(projectInfo.specificArea);
			} else {
				LinearLayout wrapper = (LinearLayout) findViewById(R.id.specific_area_wrapper);
				wrapper.setVisibility(View.GONE);
			}
			
			// set description
			if(projectInfo.description != null) {
				TextView description = (TextView) findViewById(R.id.description);
				description.setText(projectInfo.description);
			} else {
				LinearLayout wrapper = (LinearLayout) findViewById(R.id.description_wrapper);
				wrapper.setVisibility(View.GONE);
			}
			
			// set home
			if(projectInfo.home != null) {
				TextView home = (TextView) findViewById(R.id.home);
				home.setText(projectInfo.home);
			} else {
				LinearLayout wrapper = (LinearLayout) findViewById(R.id.home_wrapper);
				wrapper.setVisibility(View.GONE);
			}
		}
		
		// terms of use
		if((projectConfig.termsOfUse != null) && (projectConfig.termsOfUse.length() > 0)) {
			LinearLayout termsOfUseWrapper = (LinearLayout) findViewById(R.id.terms_of_use_wrapper);
			termsOfUseWrapper.setVisibility(View.VISIBLE);
			TextView termsOfUseCategory = (TextView) findViewById(R.id.category_terms_of_use);
			termsOfUseCategory.setText(getString(R.string.attachproject_login_category_terms_of_use) + " " + projectConfig.name);
			TextView termsOfUseText = (TextView) findViewById(R.id.project_terms_of_use);
			termsOfUseText.setText(projectConfig.termsOfUse);
		}
		
		// set account creation
		TextView creationCategory = (TextView) findViewById(R.id.category_creation);
		creationCategory.setText(getString(R.string.attachproject_login_category_creation) + " " + projectConfig.name + "?");
		TextView creationText = (TextView) findViewById(R.id.creation_action);
		Button creationSubmit = (Button) findViewById(R.id.registration_button);
		if(projectConfig.accountCreationDisabled) {
			creationText.setText(R.string.attachproject_login_header_creation_disabled);
			creationSubmit.setVisibility(View.GONE);
		} else if(projectConfig.clientAccountCreationDisabled) {
			creationText.setText(R.string.attachproject_login_header_creation_client_disabled);
			creationSubmit.setTag(false);
		} else {
			// creation in client supported
			creationText.setText(R.string.attachproject_login_header_creation_enabled);
			creationSubmit.setTag(true);
		}
		
		// set account login
		TextView loginCategory = (TextView) findViewById(R.id.category_login);
		loginCategory.setText(R.string.attachproject_login_category_login);
		if(projectConfig.userName) { // user vs. email?
			TextView idHeader = (TextView) findViewById(R.id.header_id);
			idHeader.setText(R.string.attachproject_login_header_id_name);
			EditText idInput = (EditText) findViewById(R.id.id_input);
			idInput.setInputType(InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS);
		}
	}
	
	public void login (View view) {
		new ProjectLoginAsync().execute();
	}
	
	// register button's onClick
	public void register (View view) {
		Log.d(TAG, "register: " + view.getTag());
		Boolean clientCreation = (Boolean) view.getTag();
		if (clientCreation) {
			// start intent to AttachProjectRegistrationActivity
			Intent intent = new Intent(this, AttachProjectRegistrationActivity.class);
			intent.putExtra("projectUrl", url);
			intent.putExtra("projectName", projectConfig.name);
			intent.putExtra("minPwdLength", projectConfig.minPwdLength);
			startActivity(intent);
		} else {
			// start intent to project website
			Intent i = new Intent(Intent.ACTION_VIEW);
			i.setData(Uri.parse(url));
			startActivity(i);
		}
	}
	
	// project url textview's onClick
	public void projectUrlClicked (View view) {
		// start intent to project website
		Intent i = new Intent(Intent.ACTION_VIEW);
		i.setData(Uri.parse(url));
		startActivity(i);
	}
	
	private Boolean verifyInput(String id, String pwd) {
		if(id.length() == 0) {
			Toast toast = Toast.makeText(getApplicationContext(), R.string.attachproject_login_toast_error_no_name, Toast.LENGTH_SHORT);
			toast.show();
			return false;
		}
		if(pwd.length() == 0) {
			Toast toast = Toast.makeText(getApplicationContext(), R.string.attachproject_login_toast_error_no_pwd, Toast.LENGTH_SHORT);
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
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_login_toast_ok, Toast.LENGTH_LONG);
				break;
			case BOINCErrors.ERR_BAD_PASSWD:
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_login_toast_error_wrong_pwd, Toast.LENGTH_LONG);
				break;
			case BOINCErrors.ERR_DB_NOT_FOUND:
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_login_toast_error_wrong_name, Toast.LENGTH_LONG);
				break;
			case BOINCErrors.ERR_GETHOSTBYNAME:
				toast = Toast.makeText(getApplicationContext(), R.string.attachproject_login_toast_error_no_internet, Toast.LENGTH_LONG);
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
		LinearLayout loginWrapper = (LinearLayout) findViewById(R.id.login_wrapper);
		ProgressBar loginLoading = (ProgressBar) findViewById(R.id.login_pending);
		
		if(loggingIn) {
			loginWrapper.setVisibility(View.GONE);
			loginLoading.setVisibility(View.VISIBLE);
		} else {
			loginLoading.setVisibility(View.GONE);
			loginWrapper.setVisibility(View.VISIBLE);
		}
	}
	
	private void goToMainActivity() {
		Intent intent = new Intent(this, BOINCActivity.class);
		intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP); //clear_top closes AttachProjectListActivity!
		startActivity(intent);
	}
	
	private Boolean platformSupported() {
		if(projectConfig == null) return false;
		String platformName = getString(R.string.boinc_platform_name);
		Boolean supported = false;
		for(PlatformInfo platform: projectConfig.platforms) {
			if(platform.name.equals(platformName)) {
				supported = true;
				continue;
			}
		}
		return supported;
	}
	
	private final class ProjectLoginAsync extends AsyncTask<Void, Void, Integer> {

		//private final String TAG = "ProjectLoginAsync";
		
		private String id;
		private String pwd;
		
		@Override 
		protected void onPreExecute() {
			changeLayoutLoggingIn(true);
			
			// read input data
			EditText idInput = (EditText) findViewById(R.id.id_input);
			EditText pwdInput = (EditText) findViewById(R.id.pwd_input);
			id = idInput.getText().toString();
			pwd = pwdInput.getText().toString();

			if(!verifyInput(id, pwd)) {
				changeLayoutLoggingIn(false);
				cancel(true); // cancel asynctask if validation fails
			}
			
		}
		
		@Override
		protected Integer doInBackground(Void... params) {
			
			AccountOut account = monitor.lookupCredentials(url, id, pwd);
			
			try {
				if(account.error_num == BOINCErrors.ERR_OK) {
					Boolean attach = monitor.attachProject(url, id, account.authenticator);
					if(attach) {
						return BOINCErrors.ERR_OK;
					} else {
						Log.d(TAG,"attachProject failed");
						// happens if project already attached
						return null;
					}
				} else { // error code
					return account.error_num;
				}
			} catch (Exception e) {}
			return null;
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
	
	private final class GetProjectConfig extends AsyncTask<Void, Void, Integer> {

		private final String TAG = "GetProjectConfig";
		
		@Override
		protected Integer doInBackground(Void... params) {
			try{
				if(!projectInfoPresent) { // only url string is available
					Log.d(TAG, "doInBackground() - GetProjectConfig for manual input url: " + url);
					
					if(checkProjectAlreadyAttached(url)) return R.string.attachproject_login_error_project_exists;
					
					//fetch ProjectConfig
					projectConfig = monitor.getProjectConfig(url);
				} else {
					Log.d(TAG, "doInBackground() - GetProjectConfig for list selection url: " + projectInfo.url);
					
					if(checkProjectAlreadyAttached(projectInfo.url)) return R.string.attachproject_login_error_project_exists;
					
					//fetch ProjectConfig
					projectConfig = monitor.getProjectConfig(projectInfo.url);
					
					// fetch project logo	
					loadBitmap();
				}
				
				if (projectConfig != null && projectConfig.error_num != null && projectConfig.error_num == 0) {
					return 0;
				} else { 
					Log.d(TAG,"getProjectConfig returned error num:" + projectConfig.error_num);
					return R.string.attachproject_login_error_toast;
				}
			} catch(Exception e) {
				Log.w(TAG,"error in doInBackround",e);
				return R.string.attachproject_login_error_toast;
			}
		}
		
		@Override
		protected void onPostExecute(Integer toastStringId) {
			if(toastStringId == 0) { // no error, no toast...
				populateLayout();
			} else {
				finish(toastStringId);
			}
			
		}
		
		private void loadBitmap() {
			projectLogo = null;
			try {
				URL logoUrlUrl = new URL(projectInfo.imageUrl);
				projectLogo = BitmapFactory.decodeStream(logoUrlUrl.openConnection().getInputStream());
				if(projectLogo!=null) Log.d(TAG, "logo download successful.");
			} catch (Exception e) {
				Log.w(TAG,"loadBitmap failed.",e);
			}
		}
		
		private Boolean checkProjectAlreadyAttached(String url) {
			Log.d(TAG, "check whether project with url is already attached: " + url);
			return monitor.checkProjectAttached(url);
		}
	}
}
