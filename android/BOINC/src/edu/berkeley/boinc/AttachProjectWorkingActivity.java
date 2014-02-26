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
import edu.berkeley.boinc.client.IMonitor;
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
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.rpc.AcctMgrInfo;
import edu.berkeley.boinc.utils.BOINCErrors;

public class AttachProjectWorkingActivity extends Activity{
	
	public static final int ACTION_ATTACH = 1;
	public static final int ACTION_REGISTRATION = 2;
	public static final int ACTION_ACCTMGR = 3;
	
	private IMonitor monitor;
	private Boolean mIsBound = false;
	
	private Integer timeInterval;
	
	private ArrayList<View> views = new ArrayList<View>();
	private ViewGroup anchor;
	
	private int action;
	private String projectUrl; 
	private String webRpcUrlBase; // might be empty
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
	        monitor = IMonitor.Stub.asInterface(service);
		    mIsBound = true;        
		    
		    // do desired action
		    new ProjectAccountAsync(action, projectUrl, webRpcUrlBase, id, eMail, userName, teamName, pwd, usesName, projectName).execute();
	    }

	    public void onServiceDisconnected(ComponentName className) { // This should not happen
	        monitor = null;
		    mIsBound = false;
	    }
	};
	
    @Override
    public void onCreate(Bundle savedInstanceState) {  
        super.onCreate(savedInstanceState);  
        
        // bind monitor service
        doBindService();

    	//parse information from intent extras
        try {
        	action = getIntent().getIntExtra("action", 0);
        	usesName = getIntent().getBooleanExtra("usesName", false);
        	projectUrl = getIntent().getStringExtra("projectUrl");
        	webRpcUrlBase = getIntent().getStringExtra("webRpcUrlBase");
        	projectName = getIntent().getStringExtra("projectName");
        	userName = getIntent().getStringExtra("userName");
        	teamName = getIntent().getStringExtra("teamName");
        	eMail = getIntent().getStringExtra("eMail");
        	pwd = getIntent().getStringExtra("pwd");
        	id = getIntent().getStringExtra("id");
        			
        	if(Logging.DEBUG) Log.d(Logging.TAG,"AttachProjectWorkingActivity intent extras: " + action + projectUrl + webRpcUrlBase + projectName + id + userName + teamName + eMail + usesName);
        } catch (Exception e) {
        	if(Logging.WARNING) Log.w(Logging.TAG, "AttachProjectWorkingActivity error while parsing extras", e);
        	finish(); // no point to continue without data
        }
        
        // get coniguration
        timeInterval = getResources().getInteger(R.integer.attach_step_interval_ms);
        
        // set layout
		setContentView(R.layout.attach_project_working_layout);
		anchor = (ViewGroup) findViewById(R.id.anchor);
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
		case BOINCErrors.ERR_INVALID_URL:
			stringResource = R.string.attachproject_error_invalid_url;
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
		// add flags to return to main activity and clearing all others and clear the back stack
		intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP); 
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		intent.putExtra("targetFragment", R.string.tab_projects); // make activity display projects fragment
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
		
		private Integer action;
		private String url;
		private String webRpcUrlBase; // secure ULR for lookup_account and create_account RPCs
		private String id; // used for login can be either email or user, depending on usesName
		private String email;
		private String userName;
		private String teamName;
		private String pwd;
		private Boolean usesName;
		private String projectName;
		
		public ProjectAccountAsync(Integer action, String url, String webRpcUrlBase, String id, String email, String userName, String teamName, String pwd, Boolean usesName, String projectName) {
			this.action = action;
			this.url = url;
			this.webRpcUrlBase = webRpcUrlBase;
			this.id = id; // used for login
			this.email = email;
			this.userName = userName;
			this.teamName = teamName;
			this.pwd = pwd;
			this.usesName = usesName;
			this.projectName = projectName;
			
			Log.d(Logging.TAG, "ProjectAccountAsync URL for RPCs: " + url + " ; URL for secure RPCs: " + getSecureUrlIfAvailable());
		}
		
		@Override
		protected Boolean doInBackground(Void... params) {
			if(Logging.DEBUG) Log.d(Logging.TAG,"ProjectAccountAsync doInBackground, action: " + action);
			
			//check device online
			publishProgress(new Update(false, false, R.string.attachproject_working_connect,"",0));
			try {Thread.sleep(timeInterval);} catch (Exception e){}
			if(!checkDeviceOnline()) {
				publishProgress(new Update(true, false, R.string.attachproject_working_connect, getString(R.string.attachproject_error_no_internet),-1));
				return false;
			}
			publishProgress(new Update(true, true, R.string.attachproject_working_connect,"",0));
			
			if(action == ACTION_ACCTMGR) {
			// 1st: add account manager	
				//AcctMgrRPCReply reply = null;
				int reply = -1;
				publishProgress(new Update(false, false, R.string.attachproject_working_acctmgr,"",0));
				Integer maxAttempts = getResources().getInteger(R.integer.attach_acctmgr_retries);
				Integer attemptCounter = 0;
				Boolean success = false;
				Integer err = 0;
				if(Logging.DEBUG) Log.d(Logging.TAG,"account manager with: " + url + userName + maxAttempts);
				// retry a defined number of times, if non deterministic failure occurs.
				// makes login more robust on bad network connections
				while(!success && attemptCounter < maxAttempts) {
					try {
						reply = monitor.addAcctMgrErrorNum(url, userName, pwd);
					} catch (RemoteException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
						reply = -1;
					}
					
					if(reply == -1 || reply != BOINCErrors.ERR_OK) {
						// failed
						if(reply != -1) err = reply;
						if(Logging.DEBUG) Log.d(Logging.TAG,"adding account manager failed, error code: " + err);
						if(err == -1 || err == BOINCErrors.ERR_GETHOSTBYNAME){
							// worth a retry
							attemptCounter++;
						} else {
							// not worth a retry, return
							publishProgress(new Update(true, false, R.string.attachproject_working_acctmgr, mapErrorNumToString(err),err));
							return false;
						}
					} else {
						// successful
						try {Thread.sleep(timeInterval);} catch (Exception e){}
						publishProgress(new Update(true, true, R.string.attachproject_working_acctmgr,"",0));
						success = true;
					}
				}
				// reached end of loop, check if successful
				if(!success) {
					publishProgress(new Update(true, false, R.string.attachproject_working_acctmgr, mapErrorNumToString(err),err));
					return false;
				}
			
			// 2nd: verify success by getting account manager info	
				attemptCounter = 0;
				success = false;
				publishProgress(new Update(false, false, R.string.attachproject_working_acctmgr_sync,"",0));
				// retry a defined number of times, if non deterministic failure occurs.
				// makes login more robust on bad network connections
				while(!success && attemptCounter < maxAttempts) {
					AcctMgrInfo info;
					try {
						info = monitor.getAcctMgrInfo();
					} catch (RemoteException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
						info = null;
					}
					if(Logging.DEBUG) Log.d(Logging.TAG,"acctMgrInfo: " + info.acct_mgr_url + info.acct_mgr_name + info.have_credentials);
					

					try {Thread.sleep(timeInterval);} catch (Exception e){}
					
					if(info == null) {
						// failed
						attemptCounter++;
					} else {
						// successful
						publishProgress(new Update(true, true, R.string.attachproject_working_acctmgr_sync,"",0));
						success = true;
					}
				}
				// reached end of loop, check if successful
				if(!success) {
					publishProgress(new Update(true, false, R.string.attachproject_working_acctmgr_sync, mapErrorNumToString(err),err));
					return false;
				}
				
			} else {
				// not adding account manager, either registration or attach
				// 1. get authenticator
				AccountOut account = null;
				Integer attemptCounter = 0;
				Integer maxAttempts = 0;
				Boolean success = false;
				int err = -1;
				if(action == ACTION_REGISTRATION) {
					// register account
					publishProgress(new Update(false, false, R.string.attachproject_working_register,"",0));
					maxAttempts = getResources().getInteger(R.integer.attach_creation_retries);
					if(Logging.DEBUG) Log.d(Logging.TAG,"registration with: " + url + email + userName + teamName + maxAttempts);
					// retry a defined number of times, if non deterministic failure occurs.
					// makes login more robust on bad network connections
					while(!success && attemptCounter < maxAttempts) {
						try {
							account = monitor.createAccountPolling(getSecureUrlIfAvailable(), email, userName, pwd, teamName);
						} catch (RemoteException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
							account = null;
						}
						
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
				} else if (action == ACTION_ATTACH){
					// lookup authenticator
					publishProgress(new Update(false, false, R.string.attachproject_working_verify,"",0));
					maxAttempts = getResources().getInteger(R.integer.attach_login_retries);
					if(Logging.DEBUG) Log.d(Logging.TAG,"loging with: " + url + id + usesName + maxAttempts);
					// retry a defined number of times, if non deterministic failure occurs.
					// makes login more robust on bad network connections
					while(!success && attemptCounter < maxAttempts) {
						try {
							account = monitor.lookupCredentials(getSecureUrlIfAvailable(), id, pwd, usesName);
						} catch (RemoteException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
							account = null;
						}
						
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
				
				// 2. attach project
				attemptCounter = 0;
				success = false;
				maxAttempts = getResources().getInteger(R.integer.attach_attach_retries);
				publishProgress(new Update(false, false, R.string.attachproject_working_login,"",0));
				while(!success && attemptCounter < maxAttempts) {
					Boolean attach;
					try {
						attach = monitor.attachProject(url, projectName, account.authenticator); // use standard URL for attach, i.e. not webRpcUrlBase!
					} catch (RemoteException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
						attach = false;
					}
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

		
		// returns URL to be used for RPCs.
		// either webRpcUrlBase for HTTPS, if available
		// master URL otherwise
		private String getSecureUrlIfAvailable() {
			if(webRpcUrlBase != null && !webRpcUrlBase.isEmpty()) return webRpcUrlBase;
			else return projectUrl;
		}
	}
}
