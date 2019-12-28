/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2019 University of California
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
 */
package edu.berkeley.boinc.attach;

import java.util.ArrayList;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.client.IMonitor;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.client.PersistentStorage;
import edu.berkeley.boinc.rpc.AccountIn;
import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.rpc.AcctMgrInfo;
import edu.berkeley.boinc.rpc.ProjectConfig;
import edu.berkeley.boinc.rpc.ProjectInfo;
import edu.berkeley.boinc.utils.BOINCErrors;
import edu.berkeley.boinc.utils.ErrorCodeDescription;
import edu.berkeley.boinc.utils.Logging;

import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

public class ProjectAttachService extends Service {

    // life-cycle
    private IBinder mBinder = new LocalBinder();

    public class LocalBinder extends Binder {
        ProjectAttachService getService() {
            return ProjectAttachService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "ProjectAttachService.onBind");
        }
        return mBinder;
    }

    @Override
    public void onCreate() {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "ProjectAttachService.onCreate");
        }
        doBindService();
        store = new PersistentStorage(this);
    }

    @Override
    public void onDestroy() {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "ProjectAttachService.onDestroy");
        }
        doUnbindService();
    }
    // --END-- life-cycle

    // monitor service binding
    private IMonitor monitor = null;
    private boolean mIsBound = false;

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            monitor = IMonitor.Stub.asInterface(service);
            mIsBound = true;
        }

        public void onServiceDisconnected(ComponentName className) {
            // This should not happen
            monitor = null;
            mIsBound = false;
        }
    };

    private void doBindService() {
        // Establish a connection with the service, onServiceConnected gets called when
        bindService(new Intent(this, Monitor.class), mConnection, Service.BIND_AUTO_CREATE);
    }

    private void doUnbindService() {
        if(mIsBound) {
            // Detach existing connection.
            unbindService(mConnection);
            mIsBound = false;
        }
    }
    // --END-- monitor service binding

    private PersistentStorage store;

    private ArrayList<ProjectAttachWrapper> selectedProjects = new ArrayList<>();

    public boolean projectConfigRetrievalFinished = true; // shows whether project retrieval is ongoing

    //credentials
    private String email = "";
    private String user = "";
    private String pwd = "";

    /**
     * Set credentials to be used in account RPCs.
     * Set / update prior to calling attach or register
     * Saves email and user persistently to pre-populate fields
     *
     * @param email email address of user
     * @param user  user name
     * @param pwd   password
     */
    public void setCredentials(String email, String user, String pwd) {
        this.email = email;
        this.user = user;
        this.pwd = pwd;

        store.setLastEmailAddress(email);
        store.setLastUserName(user);
    }

    /**
     * Returns last user input to be able to pre-populate fields.
     *
     * @return array of values, index 0: email address, index 1: user name
     */
    public ArrayList<String> getUserDefaultValues() {
        ArrayList<String> values = new ArrayList<>();
        values.add(store.getLastEmailAddress());
        values.add(store.getLastUserName());
        return values;
    }

    /**
     * sets selected projects and downloads their configuration files
     * configuration download in new thread, returns immediately.
     * Check projectConfigRetrievalFinished to see whether job finished.
     *
     * @param selected list of selected projects
     * @return success
     */
    public boolean setSelectedProjects(ArrayList<ProjectInfo> selected) {
        if(!projectConfigRetrievalFinished) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "ProjectAttachService.setSelectedProjects: stop, async task already running.");
            }
            return false;
        }

        selectedProjects.clear();
        for(ProjectInfo tmp : selected) {
            selectedProjects.add(new ProjectAttachWrapper(tmp));
        }

        if(mIsBound) {
            new GetProjectConfigsAsync().execute();
        }
        else {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "ProjectAttachService.setSelectedProjects: could not load configuration files, monitor not bound.");
            }
            return false;
        }

        if(Logging.DEBUG) {
            Log.d(Logging.TAG,
                  "ProjectAttachService.setSelectedProjects: number of selected projects: " + selectedProjects.size());
        }
        return true;
    }

    /**
     * sets single selected project with URL inserted manually, not chosen from list.
     * Starts configuration download in new thread and returns immediately.
     * Check projectConfigRetrievalFinished to see whether job finished.
     *
     * @param url URL of project
     * @return success
     */
    public boolean setManuallySelectedProject(String url) {
        if(!projectConfigRetrievalFinished) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "ProjectAttachService.setManuallySelectedProject: stop, async task already running.");
            }
            return false;
        }

        selectedProjects.clear();
        selectedProjects.add(new ProjectAttachWrapper(url));

        // get projectConfig
        if(mIsBound) {
            new GetProjectConfigsAsync().execute();
        }
        else {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "ProjectAttachService.setManuallySelectedProject: could not load configuration file, monitor not bound.");
            }
            return false;
        }

        if(Logging.DEBUG) {
            Log.d(Logging.TAG,
                  "ProjectAttachService.setManuallySelectedProject: url of selected project: " + url + ", list size: " +
                  selectedProjects.size());
        }
        return true;

    }

    public int getNumberSelectedProjects() {
        return selectedProjects.size();
    }

    public ArrayList<ProjectAttachWrapper> getSelectedProjects() {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG,
                  "ProjectAttachService.getSelectedProjects: returning list of size: " + selectedProjects.size());
        }
        return this.selectedProjects;
    }

    /**
     * Returns selected but untried project to be attached
     *
     * @return project or null if no more untried projects
     */
    public ProjectAttachWrapper getNextSelectedProject() {
        for(ProjectAttachWrapper tmp : selectedProjects) {
            if(tmp.result == ProjectAttachWrapper.RESULT_READY ||
               tmp.result == ProjectAttachWrapper.RESULT_UNINITIALIZED) {
                return tmp;
            }
        }
        return null;
    }

    /**
     * Checks user input, e.g. length of input. Shows an error toast if problem detected
     *
     * @param email email address of user
     * @param user  user name
     * @param pwd   password
     * @return true if input verified
     */
    public Boolean verifyInput(String email, String user, String pwd) {
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

        if(stringResource != 0) {
            Toast toast = Toast.makeText(getApplicationContext(), stringResource, Toast.LENGTH_LONG);
            toast.show();
            return false;
        }
        else {
            return true;
        }
    }

    /**
     * Returns true as long as there have been unresolved conflicts.
     *
     * @return indicator whether conflicts exist
     */
    public boolean unresolvedConflicts() {
        for(ProjectAttachWrapper project : selectedProjects) {
            if(project.result != ProjectAttachWrapper.RESULT_SUCCESS) {
                return true;
            }
        }
        return false;
    }

    /**
     * Attempts attach of account manager with credentials provided as parameter.
     * Does not require to select project or set credentials beforehand.
     *
     * @param url  acct mgr url
     * @param name user name
     * @param pwd  password
     * @return result code, see BOINCErrors
     */
    public ErrorCodeDescription attachAcctMgr(String url, String name, String pwd) {

        ErrorCodeDescription reply = new ErrorCodeDescription();
        Integer maxAttempts = getResources().getInteger(R.integer.attach_acctmgr_retries);
        Integer attemptCounter = 0;
        Boolean retry = true;
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "account manager with: " + url + name + maxAttempts);
        }
        // retry a defined number of times, if non deterministic failure occurs.
        // makes login more robust on bad network connections
        while(retry && attemptCounter < maxAttempts) {
            try {
                reply = monitor.addAcctMgrErrorNum(url, name, pwd);
            }
            catch(RemoteException e) {
                if(Logging.ERROR) {
                    Log.e(Logging.TAG, "ProjectAttachService.attachAcctMgr error: ", e);
                }
            }

            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "ProjectAttachService.attachAcctMgr returned: " + reply);
            }
            switch(reply.code) {
                case BOINCErrors.ERR_GETHOSTBYNAME: // no internet
                    attemptCounter++; // limit number of retries
                    break;
                case BOINCErrors.ERR_CONNECT: // connection problems
                    attemptCounter++; // limit number of retries
                    break;
                case BOINCErrors.ERR_HTTP_TRANSIENT: // connection problems
                    attemptCounter++; // limit number of retries
                    break;
                case BOINCErrors.ERR_RETRY: // client currently busy with another HTTP request, retry unlimited
                    break;
                default: // success or final error, stop retrying
                    retry = false;
                    break;
            }
            if(retry) {
                try {
                    Thread.sleep(getResources().getInteger(R.integer.attach_step_interval_ms));
                }
                catch(Exception ignored) {
                }
            }
        }

        if(reply.code != BOINCErrors.ERR_OK || (reply.description != null && !reply.description.isEmpty())) {
            return reply;
        }

        AcctMgrInfo info = null;
        try {
            info = monitor.getAcctMgrInfo();
        }
        catch(RemoteException e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "ProjectAttachService.attachAcctMgr error: ", e);
            }
        }
        if(info == null) {
            return new ErrorCodeDescription(-1);
        }

        if(Logging.DEBUG) {
            Log.d(Logging.TAG,
                  "ProjectAttachService.attachAcctMgr successful: " + info.acct_mgr_url + info.acct_mgr_name +
                  info.have_credentials);
        }
        return new ErrorCodeDescription(BOINCErrors.ERR_OK);
    }

    public class ProjectAttachWrapper {
        public String url; // URL, manually inserted, or from projectInfo
        public ProjectInfo info; // chosen from list
        public String name; // name of project in debug messages, do not use otherwise!
        public ProjectConfig config; // has to be downloaded, available if RESULT_READY
        public int result = RESULT_UNINITIALIZED;

        // config not downloaded yet, download!
        public static final int RESULT_UNINITIALIZED = 0;
        // config is available, project is ready to be attached
        public static final int RESULT_READY = 1;
        // ongoing attach
        public static final int RESULT_ONGOING = 2;
        // successful, -X otherwise
        public static final int RESULT_SUCCESS = 3;
        public static final int RESULT_UNDEFINED = -1;
        // registration failed, either password wrong or ID taken
        public static final int RESULT_NAME_NOT_UNIQUE = -2;
        // login failed (creation disabled or login button pressed), password wrong
        public static final int RESULT_BAD_PASSWORD = -3;
        // login failed (creation disabled or login button pressed), user does not exist
        public static final int RESULT_UNKNOWN_USER = -4;
        public static final int RESULT_REQUIRES_TOS_AGGREEMENT = -5;
        // download of configuration failed, but required for attach (retry?)
        public static final int RESULT_CONFIG_DOWNLOAD_FAILED = -6;


        public ProjectAttachWrapper(ProjectInfo info) {
            this.info = info;
            this.name = info.name;
            this.url = info.url;
        }

        public ProjectAttachWrapper(String url) {
            this.url = url;
            this.name = url;
        }

        public String getResultDescription() {
            switch(result) {
                case RESULT_UNINITIALIZED:
                    return getString(R.string.attachproject_login_loading);
                case RESULT_READY:
                    return getString(R.string.attachproject_credential_input_sing_desc);
                case RESULT_ONGOING:
                    return getString(R.string.attachproject_working_attaching) + " " + this.name + "...";
                case RESULT_UNDEFINED:
                    return getString(R.string.attachproject_conflict_undefined);
                case RESULT_NAME_NOT_UNIQUE:
                    return getString(R.string.attachproject_conflict_not_unique);
                case RESULT_BAD_PASSWORD:
                    return getString(R.string.attachproject_conflict_bad_password);
                case RESULT_UNKNOWN_USER:
                    if(config.clientAccountCreationDisabled) {
                        return getString(R.string.attachproject_conflict_unknown_user_creation_disabled);
                    }
                    else {
                        return getString(R.string.attachproject_conflict_unknown_user);
                    }
                case RESULT_CONFIG_DOWNLOAD_FAILED:
                    return getString(R.string.attachproject_conflict_undefined);
            }
            return "";
        }

        /**
         * Attaches this project to BOINC client.
         * Account lookup/registration using credentials set at service.
         * <p>
         * Using registration RPC if client side registration is enabled,
         * succeeds also if account exists and password is correct.
         * <p>
         * Using login RPC if client side registration is disabled.
         * <p>
         * Attaches project if account lookup succeeded.
         * <p>
         * Retries in case of non-deterministic errors
         * Long-running and network communication, do not execute in UI thread.
         *
         * @return returns status conflict
         */
        public int lookupAndAttach(boolean forceLookup) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "ProjectAttachWrapper.attach: attempting: " + name);
            }

            // check if project config is loaded, return if not.
            // activity needs to check, wait and re-try
            if(result == RESULT_UNINITIALIZED || !projectConfigRetrievalFinished || config == null) {
                if(Logging.ERROR) {
                    Log.e(Logging.TAG, "ProjectAttachWrapper.attach: no projectConfig for: " + name);
                }
                result = RESULT_UNDEFINED;
                return RESULT_UNDEFINED;
            }

            result = RESULT_ONGOING;

            // get credentials
            AccountOut statusCredentials;
            // check if project allows registration
            if(forceLookup || config.clientAccountCreationDisabled) {
                // registration disabled, e.g. WCG
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG,
                          "AttachProjectAsyncTask: account creation disabled, try login. for: " + config.name);
                }
                statusCredentials = login();
            }
            else {
                // registration enabled
                statusCredentials = register();
            }
            if(Logging.DEBUG) {
                Log.d(Logging.TAG,
                      "AttachProjectAsyncTask: retrieving credentials returned: " + statusCredentials.error_num + ":" +
                      statusCredentials.error_msg + ". for: " + config.name);
            }

            // check success
            if(statusCredentials == null) {
                if(Logging.ERROR) {
                    Log.e(Logging.TAG, "AttachProjectAsyncTask: credential retrieval failed, is null, for: " + name);
                }
                result = RESULT_UNDEFINED;
                return RESULT_UNDEFINED;
            }
            else if(statusCredentials.error_num != BOINCErrors.ERR_OK) {
                if(Logging.ERROR) {
                    Log.e(Logging.TAG, "AttachProjectAsyncTask: credential retrieval failed, returned error: " +
                                       statusCredentials.error_num);
                }
                switch(statusCredentials.error_num) {
                    case BOINCErrors.ERR_DB_NOT_UNIQUE:
                        result = RESULT_NAME_NOT_UNIQUE;
                        return RESULT_NAME_NOT_UNIQUE;
                    case BOINCErrors.ERR_BAD_PASSWD:
                        result = RESULT_BAD_PASSWORD;
                        return RESULT_BAD_PASSWORD;
                    case BOINCErrors.ERR_DB_NOT_FOUND:
                        result = RESULT_UNKNOWN_USER;
                        return RESULT_UNKNOWN_USER;
                    default:
                        if(Logging.WARNING) {
                            Log.w(Logging.TAG, "AttachProjectAsyncTask: unable to map error number, returned error: " +
                                               statusCredentials.error_num);
                        }
                        result = RESULT_UNDEFINED;
                        return RESULT_UNDEFINED;
                }
            }

            // attach project;
            boolean statusAttach = attach(statusCredentials.authenticator);
            if(Logging.DEBUG) {
                Log.d(Logging.TAG,
                      "AttachProjectAsyncTask: attach returned: " + statusAttach + ". for: " + config.name);
            }

            if(!statusAttach) {
                result = RESULT_UNDEFINED;
                return RESULT_UNDEFINED;
            }

            result = RESULT_SUCCESS;
            return RESULT_SUCCESS;
        }

        /**
         * Attempts account registration with the credentials previously set in service.
         * Registration also succeeds if account exists and password is correct.
         * <p>
         * Retries in case of non-deterministic errors
         * Long-running and network communication, do not execute in UI thread.
         *
         * @return credentials
         */
        public AccountOut register() {
            AccountOut credentials = null;
            boolean retry = true;
            Integer attemptCounter = 0;
            Integer maxAttempts = getResources().getInteger(R.integer.attach_creation_retries);
            // retry a defined number of times, if non deterministic failure occurs.
            // makes login more robust on bad network connections
            while(retry && attemptCounter < maxAttempts) {
                if(mIsBound) {
                    try {
                        credentials = monitor.createAccountPolling(getAccountIn(email, user, pwd));
                    }
                    catch(RemoteException e) {
                        if(Logging.ERROR) {
                            Log.e(Logging.TAG, "ProjectAttachService.register error: ", e);
                        }
                    }
                }
                if(credentials == null) {
                    // call failed
                    if(Logging.WARNING) {
                        Log.w(Logging.TAG, "ProjectAttachWrapper.register register: auth null, retry...");
                    }
                    attemptCounter++; // limit number of retries
                }
                else {
                    if(Logging.DEBUG) {
                        Log.d(Logging.TAG,
                              "ProjectAttachWrapper.register returned: " + config.error_num + " for " + name);
                    }
                    switch(config.error_num) {
                        case BOINCErrors.ERR_GETHOSTBYNAME: // no internet
                            attemptCounter++; // limit number of retries
                            break;
                        case BOINCErrors.ERR_CONNECT: // connection problems
                            attemptCounter++; // limit number of retries
                            break;
                        case BOINCErrors.ERR_HTTP_TRANSIENT: // connection problems
                            attemptCounter++; // limit number of retries
                            break;
                        case BOINCErrors.ERR_RETRY: // client currently busy with another HTTP request, retry unlimited
                            break;
                        default: // success or final error, stop retrying
                            retry = false;
                            break;
                    }
                }
            }

            return credentials;
        }

        /**
         * Attempts account lookup with the credentials previously set in service.
         * <p>
         * Retries in case of non-deterministic errors
         * Long-running and network communication, do not execute in UI thread.
         *
         * @return credentials
         */
        public AccountOut login() {
            AccountOut credentials = null;
            boolean retry = true;
            Integer attemptCounter = 0;
            Integer maxAttempts = getResources().getInteger(R.integer.attach_login_retries);
            // retry a defined number of times, if non deterministic failure occurs.
            // makes login more robust on bad network connections
            while(retry && attemptCounter < maxAttempts) {
                if(mIsBound) {
                    try {
                        credentials = monitor.lookupCredentials(getAccountIn(email, user, pwd));
                    }
                    catch(RemoteException e) {
                        if(Logging.ERROR) {
                            Log.e(Logging.TAG, "ProjectAttachService.login error: ", e);
                        }
                    }
                }
                if(credentials == null) {
                    // call failed
                    if(Logging.WARNING) {
                        Log.w(Logging.TAG, "ProjectAttachWrapper.login failed: auth null, retry...");
                    }
                    attemptCounter++; // limit number of retries
                }
                else {
                    if(Logging.DEBUG) {
                        Log.d(Logging.TAG, "ProjectAttachWrapper.login returned: " + config.error_num + " for " + name);
                    }
                    switch(config.error_num) {
                        case BOINCErrors.ERR_GETHOSTBYNAME: // no internet
                            attemptCounter++; // limit number of retries
                            break;
                        case BOINCErrors.ERR_CONNECT: // connection problems
                            attemptCounter++; // limit number of retries
                            break;
                        case BOINCErrors.ERR_HTTP_TRANSIENT: // connection problems
                            attemptCounter++; // limit number of retries
                            break;
                        case BOINCErrors.ERR_RETRY: // client currently busy with another HTTP request, retry unlimited
                            break;
                        default: // success or final error, stop retrying
                            retry = false;
                            break;
                    }
                }

                if(retry) {
                    try {
                        Thread.sleep(getResources().getInteger(R.integer.attach_step_interval_ms));
                    }
                    catch(Exception ignored) {
                    }
                }
            }

            return credentials;
        }

        private boolean attach(String authenticator) {
            if(mIsBound) {
                try {
                    return monitor.attachProject(config.masterUrl, config.name, authenticator);
                }
                catch(RemoteException e) {
                    if(Logging.ERROR) {
                        Log.e(Logging.TAG, "ProjectAttachService.attach error: ", e);
                    }
                }
            }
            return false;
        }

        private AccountIn getAccountIn(String email, String user, String pwd) {
            return new AccountIn(config.getSecureUrlIfAvailable(), email, user, config.usesName, pwd, "");
        }
    }

    private class GetProjectConfigsAsync extends AsyncTask<Void, Void, Void> {

        Integer maxAttempts = getResources().getInteger(R.integer.attach_get_project_config_retries);

        @Override
        protected void onPreExecute() {
            projectConfigRetrievalFinished = false;
            super.onPreExecute();
        }

        @Override
        protected Void doInBackground(Void... params) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "ProjectAttachService.GetProjectConfigAsync: number of selected projects: " +
                                   selectedProjects.size());
            }
            for(ProjectAttachWrapper tmp : selectedProjects) {
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG,
                          "ProjectAttachService.GetProjectConfigAsync: configuration download started for: " +
                          tmp.name + " with URL: " + tmp.url);
                }
                ProjectConfig config = getProjectConfig(tmp.url);
                if(config != null && config.error_num == BOINCErrors.ERR_OK) {
                    if(Logging.DEBUG) {
                        Log.d(Logging.TAG,
                              "ProjectAttachService.GetProjectConfigAsync: configuration download succeeded for: " +
                              tmp.name);
                    }
                    tmp.config = config;
                    tmp.name = config.name;
                    tmp.result = ProjectAttachWrapper.RESULT_READY;
                }
                else {
                    // error occurred
                    if(Logging.WARNING) {
                        Log.w(Logging.TAG,
                              "ProjectAttachService.GetProjectConfigAsync: could not load configuration for: " +
                              tmp.name);
                    }
                    tmp.result = ProjectAttachWrapper.RESULT_CONFIG_DOWNLOAD_FAILED;
                }
            }
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "ProjectAttachService.GetProjectConfigAsync: end.");
            }
            return null;
        }

        private ProjectConfig getProjectConfig(String url) {
            ProjectConfig config = null;
            boolean retry = true;
            Integer attemptCounter = 0;
            // retry a defined number of times, if non deterministic failure occurs.
            // makes login more robust on bad network connections
            while(retry && attemptCounter < maxAttempts) {
                if(mIsBound) {
                    try {
                        config = monitor.getProjectConfigPolling(url);
                    }
                    catch(RemoteException e) {
                        if(Logging.ERROR) {
                            Log.e(Logging.TAG, "ProjectAttachService.getProjectConfig error: ", e);
                        }
                    }
                }
                if(config == null) {
                    // call failed
                    if(Logging.WARNING) {
                        Log.w(Logging.TAG,
                              "ProjectAttachWrapper.getProjectConfig failed: config null, mIsBound: " + mIsBound +
                              " for " + url + ". retry...");
                    }
                    attemptCounter++; // limit number of retries
                }
                else {
                    if(Logging.DEBUG) {
                        Log.d(Logging.TAG,
                              "GetProjectConfigsAsync.getProjectConfig returned: " + config.error_num + " for " + url);
                    }
                    switch(config.error_num) {
                        case BOINCErrors.ERR_GETHOSTBYNAME: // no internet
                            attemptCounter++; // limit number of retries
                            break;
						/* disable retries fo ERR_CONNECT because timeout can be very long (over 60sec)
					case BOINCErrors.ERR_CONNECT: // connection problems
						attemptCounter++; // limit number of retries
						break;*/
                        case BOINCErrors.ERR_HTTP_TRANSIENT: // connection problems
                            attemptCounter++; // limit number of retries
                            break;
                        case BOINCErrors.ERR_RETRY: // client currently busy with another HTTP request, retry unlimited
                            break;
                        default: // success or final error, stop retrying
                            retry = false;
                            break;
                    }
                }

                if(retry) {
                    try {
                        Thread.sleep(getResources().getInteger(R.integer.attach_step_interval_ms));
                    }
                    catch(Exception ignored) {
                    }
                }
            }
            return config;
        }

        @Override
        protected void onPostExecute(Void result) {
            projectConfigRetrievalFinished = true;
            super.onPostExecute(result);
        }

    }
}
