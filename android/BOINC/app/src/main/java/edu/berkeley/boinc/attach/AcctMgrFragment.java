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

import android.app.Activity;
import android.app.Dialog;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
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

import androidx.annotation.NonNull;
import androidx.fragment.app.DialogFragment;

import org.apache.commons.lang3.StringUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.client.IMonitor;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.AccountManager;
import edu.berkeley.boinc.utils.BOINCErrors;
import edu.berkeley.boinc.utils.ErrorCodeDescription;
import edu.berkeley.boinc.utils.Logging;

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

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
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
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                AccountManagerSpinner accountManagerSpinner = (AccountManagerSpinner) urlSpinner.getSelectedItem();
                urlInput.setText(accountManagerSpinner.getUrl());
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {
            }
        });

        continueB.setOnClickListener(view -> {
            if (Logging.DEBUG) Log.d(Logging.TAG, "AcctMgrFragment continue clicked");
            if (!checkDeviceOnline()) return;
            if (asIsBound) {
                // get user input
                String url = urlInput.getText().toString();
                String name = nameInput.getText().toString();
                String pwd = pwdInput.getText().toString();

                // verify input
                int res = verifyInput(url, name, pwd);
                if (res != 0) {
                    warning.setText(res);
                    warning.setVisibility(View.VISIBLE);
                    return;
                }

                // adapt layout
                continueB.setVisibility(View.GONE);
                warning.setVisibility(View.GONE);
                ongoingWrapper.setVisibility(View.VISIBLE);

                asyncTask = new AttachProjectAsyncTask();
                asyncTask.execute(url, name, pwd);

            } else if (Logging.DEBUG)
                Log.d(Logging.TAG, "AcctMgrFragment service not bound, do nothing...");
        });

        doBindService();

        return v;
    }

    @Override
    public void onDestroyView() {
        doUnbindService();
        if (asyncTask != null)
            asyncTask.cancel(true);
        super.onDestroyView();
    }

    @NonNull
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Dialog dialog = super.onCreateDialog(savedInstanceState);

        // request a window without the title
        dialog.getWindow().requestFeature(Window.FEATURE_NO_TITLE);
        return dialog;
    }

    void setReturnToMainActivity() {
        returnToMainActivity = true;
    }

    private int verifyInput(String url, String name, String pwd) {
        int stringResource = 0;

        // check input
        if (url.isEmpty()) {
            stringResource = R.string.attachproject_error_no_url;
        } else if (name.isEmpty()) {
            stringResource = R.string.attachproject_error_no_name;
        } else if (pwd.isEmpty()) {
            stringResource = R.string.attachproject_error_no_pwd;
        }

        return stringResource;
    }

    // check whether device is online before starting connection attempt
    // as needed for AttachProjectLoginActivity (retrieval of ProjectConfig)
    // note: available internet does not imply connection to project server
    // is possible!
    private boolean checkDeviceOnline() {
        final Activity activity = getActivity();
        assert activity != null;
        final ConnectivityManager connectivityManager =
                (ConnectivityManager) activity.getSystemService(Context.CONNECTIVITY_SERVICE);
        assert connectivityManager != null;
        final boolean online;
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            final NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
            online = activeNetworkInfo != null && activeNetworkInfo.isConnectedOrConnecting();
        } else {
            online = connectivityManager.getActiveNetwork() != null;
        }
        if (!online) {
            Toast toast = Toast.makeText(getActivity(), R.string.attachproject_list_no_internet, Toast.LENGTH_SHORT);
            toast.show();
            if (Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectListActivity not online, stop!");
        }
        return online;
    }

    private ServiceConnection mMonitorConnection = new ServiceConnection() {
        private void fillAdapterData() {
            List<AccountManager> accountManagers = null;
            if (mIsBound) {
                try {
                    accountManagers = monitor.getAccountManagers();
                } catch (Exception e) {
                    if (Logging.ERROR) Log.e(Logging.TAG, "AcctMgrFragment onCreateView() error: " + e);
                    accountManagers = Collections.emptyList();
                }
                List<AccountManagerSpinner> adapterData = new ArrayList<>();
                for (AccountManager accountManager : accountManagers) {
                    adapterData.add(new AccountManagerSpinner(accountManager.getName(), accountManager.getUrl()));
                }
                final ArrayAdapter adapter = new ArrayAdapter(getActivity(),
                                                              android.R.layout.simple_spinner_item,
                                                              adapterData);
                adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                urlSpinner.setAdapter(adapter);
            }
        }

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            monitor = IMonitor.Stub.asInterface(service);
            mIsBound = true;
            fillAdapterData();
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            // This should not happen
            monitor = null;
            mIsBound = false;

            Log.e(Logging.TAG, "BOINCActivity onServiceDisconnected");
        }
    };

    private ServiceConnection mASConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            attachService = ((ProjectAttachService.LocalBinder) service).getService();
            asIsBound = true;
        }

        @Override
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

    private class AttachProjectAsyncTask extends AsyncTask<String, Void, ErrorCodeDescription> {
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
            if (result.getCode() == BOINCErrors.ERR_OK && StringUtils.isEmpty(result.getDescription())) {
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
                if (StringUtils.isNotEmpty(result.getDescription())) {
                    warning.setText(result.getDescription());
                } else {
                    warning.setText(mapErrorNumToString(result.getCode()));
                }
            }
        }
    }
}
