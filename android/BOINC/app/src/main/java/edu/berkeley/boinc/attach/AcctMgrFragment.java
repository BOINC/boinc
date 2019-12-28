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

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.client.IMonitor;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.AccountManager;
import edu.berkeley.boinc.utils.*;

import android.app.Dialog;
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
import android.support.v4.app.DialogFragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

public class AcctMgrFragment extends DialogFragment {

    // services
    private IMonitor monitor = null;
    private boolean mIsBound = false;
    private ProjectAttachService attachService = null;
    private boolean asIsBound = false;

    private Spinner urlSpinner;
    private EditText urlInput;
    private EditText nameInput;
    private EditText pwdInput;
    private TextView warning;
    private LinearLayout ongoingWrapper;
    private Button continueB;

    private boolean returnToMainActivity = false;

    private AttachProjectAsyncTask asyncTask;


    private void fillAdapterData() {
        ArrayList<AccountManager> accountManagers = null;
        if (mIsBound) {
            try {
                accountManagers = (ArrayList<AccountManager>) monitor.getAccountManagers();
            } catch (Exception e) {
                if (Logging.ERROR) Log.e(Logging.TAG, "AcctMgrFragment onCreateView() error: " + e);
            }
            List<AccountManagerSpinner> adapterData = new ArrayList<>();
            for (AccountManager accountManager : accountManagers) {
                adapterData.add(new AccountManagerSpinner(accountManager.name, accountManager.url));
            }
            ArrayAdapter adapter = new ArrayAdapter(getActivity(), android.R.layout.simple_spinner_item, adapterData);
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            urlSpinner.setAdapter(adapter);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "AcctMgrFragment onCreateView");
        }
        View v = inflater.inflate(R.layout.attach_project_acctmgr_dialog, container, false);

        urlSpinner = v.findViewById(R.id.url_spinner);
        urlInput = v.findViewById(R.id.url_input);
        nameInput = v.findViewById(R.id.name_input);
        pwdInput = v.findViewById(R.id.pwd_input);
        warning = v.findViewById(R.id.warning);
        ongoingWrapper = v.findViewById(R.id.ongoing_wrapper);
        continueB = v.findViewById(R.id.continue_button);

        // change url text field on url spinner change
        urlSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                AccountManagerSpinner accountManagerSpinner = (AccountManagerSpinner) urlSpinner.getSelectedItem();
                urlInput.setText(accountManagerSpinner.url);
            }

            public void onNothingSelected(AdapterView<?> adapterView) {
                return;
            }
        });

        continueB.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                if (Logging.DEBUG) Log.d(Logging.TAG, "AcctMgrFragment continue clicked");
                if (!checkDeviceOnline()) return;
                if (asIsBound) {

                    // get user input
                    String url = urlInput.getText().toString();
                    String name = nameInput.getText().toString();
                    String pwd = pwdInput.getText().toString();

                    // verify input
                    int res;
                    if ((res = verifyInput(url, name, pwd)) != 0) {
                        warning.setText(res);
                        warning.setVisibility(View.VISIBLE);
                        return;
                    }

                    // adapt layout
                    continueB.setVisibility(View.GONE);
                    warning.setVisibility(View.GONE);
                    ongoingWrapper.setVisibility(View.VISIBLE);

                    String[] params = new String[3];
                    params[0] = url;
                    params[1] = name;
                    params[2] = pwd;
                    asyncTask = new AttachProjectAsyncTask();
                    asyncTask.execute(params);

                } else if (Logging.DEBUG)
                    Log.d(Logging.TAG, "AcctMgrFragment service not bound, do nothing...");
            }

        });

        doBindService();

        return v;
    }

    @Override
    public void onDestroyView() {
        doUnbindService();
        if (asyncTask != null) asyncTask.cancel(true);
        super.onDestroyView();
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Dialog dialog = super.onCreateDialog(savedInstanceState);

        // request a window without the title
        dialog.getWindow().requestFeature(Window.FEATURE_NO_TITLE);
        return dialog;
    }

    public void setReturnToMainActivity() {
        returnToMainActivity = true;
    }

    private int verifyInput(String url, String name, String pwd) {
        int stringResource = 0;

        // check input
        if (url.length() == 0) {
            stringResource = R.string.attachproject_error_no_url;
        } else if (name.length() == 0) {
            stringResource = R.string.attachproject_error_no_name;
        } else if (pwd.length() == 0) {
            stringResource = R.string.attachproject_error_no_pwd;
        }

        return stringResource;
    }

    // check whether device is online before starting connection attempt
    // as needed for AttachProjectLoginActivity (retrieval of ProjectConfig)
    // note: available internet does not imply connection to project server
    // is possible!
    private Boolean checkDeviceOnline() {
        ConnectivityManager connectivityManager = (ConnectivityManager) getActivity().getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
        Boolean online = activeNetworkInfo != null && activeNetworkInfo.isConnectedOrConnecting();
        if (!online) {
            Toast toast = Toast.makeText(getActivity(), R.string.attachproject_list_no_internet, Toast.LENGTH_SHORT);
            toast.show();
            if (Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectListActivity not online, stop!");
        }
        return online;
    }

    private ServiceConnection mMonitorConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            monitor = IMonitor.Stub.asInterface(service);
            mIsBound = true;
            fillAdapterData();
        }

        public void onServiceDisconnected(ComponentName className) {
            // This should not happen
            monitor = null;
            mIsBound = false;

            Log.e(Logging.TAG, "BOINCActivity onServiceDisconnected");
        }
    };

    private ServiceConnection mASConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            attachService = ((ProjectAttachService.LocalBinder) service).getService();
            asIsBound = true;
        }

        public void onServiceDisconnected(ComponentName className) {
            // This should not happen
            attachService = null;
            asIsBound = false;
        }
    };

    private void doBindService() {
        // start service to allow setForeground later on...
        getActivity().startService(new Intent(getActivity(), Monitor.class));
        // Establish a connection with the service, onServiceConnected gets called when
        getActivity().bindService(new Intent(getActivity(), Monitor.class), mMonitorConnection, Service.BIND_AUTO_CREATE);

        // bind to attach service
        getActivity().bindService(new Intent(getActivity(), ProjectAttachService.class), mASConnection, Service.BIND_AUTO_CREATE);
    }

    private void doUnbindService() {

        if (mIsBound) {
            // Detach existing connection.
            getActivity().unbindService(mMonitorConnection);
            mIsBound = false;
        }

        if (asIsBound) {
            // Detach existing connection.
            getActivity().unbindService(mASConnection);
            asIsBound = false;
        }
    }

    private String mapErrorNumToString(int code) {
        if (Logging.DEBUG) Log.d(Logging.TAG, "mapErrorNumToString for error: " + code);
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
                stringResource = R.string.attachproject_error_email_in_use;
                break;
            case BOINCErrors.ERR_PROJECT_DOWN:
                stringResource = R.string.attachproject_error_project_down;
                break;
            case BOINCErrors.ERR_BAD_EMAIL_ADDR:
                stringResource = R.string.attachproject_error_email_bad_syntax;
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

    private class AttachProjectAsyncTask extends AsyncTask<String, Void, ErrorCodeDescription> {

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
        }

        @Override
        protected ErrorCodeDescription doInBackground(String... arg0) {

            String url = arg0[0];
            String name = arg0[1];
            String pwd = arg0[2];

            return attachService.attachAcctMgr(url, name, pwd);
        }

        @Override
        protected void onPostExecute(ErrorCodeDescription result) {
            super.onPostExecute(result);
            if (Logging.DEBUG)
                Log.d(Logging.TAG, "AcctMgrFragment.AttachProjectAsyncTask onPostExecute, returned: " + result);
            if (result.code == BOINCErrors.ERR_OK && (result.description == null || result.description.isEmpty())) {
                dismiss();
                if (returnToMainActivity) {
                    if (Logging.DEBUG)
                        Log.d(Logging.TAG, "AcctMgrFragment.AttachProjectAsyncTask onPostExecute, start main activity");
                    Intent intent = new Intent(getActivity(), BOINCActivity.class);
                    // add flags to return to main activity and clearing all others and clear the back stack
                    intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    intent.putExtra("targetFragment", R.string.tab_projects); // make activity display projects fragment
                    startActivity(intent);
                }
            } else {
                ongoingWrapper.setVisibility(View.GONE);
                continueB.setVisibility(View.VISIBLE);
                warning.setVisibility(View.VISIBLE);
                if (result.description != null && !result.description.isEmpty()) {
                    warning.setText(result.description);
                } else {
                    warning.setText(mapErrorNumToString(result.code));
                }
            }
        }
    }

}
