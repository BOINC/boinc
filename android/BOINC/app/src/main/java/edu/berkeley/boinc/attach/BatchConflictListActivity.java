/*
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
 */

package edu.berkeley.boinc.attach;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.*;

import java.util.ArrayList;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.attach.IndividualCredentialInputFragment.IndividualCredentialInputFragmentListener;
import edu.berkeley.boinc.attach.ProjectAttachService.ProjectAttachWrapper;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.View;
import android.widget.ListView;
import android.widget.TextView;

public class BatchConflictListActivity extends FragmentActivity implements IndividualCredentialInputFragmentListener {

    private ListView lv;
    private BatchConflictListAdapter listAdapter;
    private ArrayList<ProjectAttachWrapper> results = new ArrayList<>();

    private ProjectAttachService attachService = null;
    private boolean asIsBound = false;

    private String manualUrl;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BatchConflictListActivity onCreate");
        }
        doBindService();
        // setup layout
        setContentView(R.layout.attach_project_batch_conflicts_layout);
        lv = findViewById(R.id.listview);
        // adapt text
        Intent intent = getIntent();
        Boolean conflicts = intent.getBooleanExtra("conflicts", false);
        manualUrl = intent.getStringExtra("manualUrl");
        TextView title = findViewById(R.id.desc);
        if(conflicts) {
            title.setText(R.string.attachproject_conflicts_desc);
        }
        else {
            title.setText(R.string.attachproject_credential_input_desc);
        }
    }

    @Override
    protected void onDestroy() {
        if(Logging.VERBOSE) {
            Log.v(Logging.TAG, "BatchConflictListActivity onDestroy");
        }
        doUnbindService();
        super.onDestroy();
    }

    @Override
    public void onResume() {
        registerReceiver(mClientStatusChangeRec, ifcsc);
        super.onResume();
    }

    @Override
    public void onPause() {
        unregisterReceiver(mClientStatusChangeRec);
        super.onPause();
    }

    // not related to client status change, just used to implement cyclic checking of
    // results.
    private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if(Logging.VERBOSE) {
                Log.d(Logging.TAG, "BatchConflictListActivity ClientStatusChange - onReceive()");
            }
            if(asIsBound) {
                listAdapter.notifyDataSetChanged();
            }
        }
    };
    private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");


    // triggered by finish button
    public void finishClicked(View v) {
        // finally, start BOINCActivity
        Intent intent = new Intent(this, BOINCActivity.class);
        // add flags to return to main activity and clearing all others and clear the back stack
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra("targetFragment", R.string.tab_projects); // make activity display projects fragment
        startActivity(intent);
    }

    private ServiceConnection mASConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            attachService = ((ProjectAttachService.LocalBinder) service).getService();
            asIsBound = true;

            // set data, if manual url
            if(manualUrl != null && !manualUrl.isEmpty()) {
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "BatchConflictListActivity manual URL found: " + manualUrl);
                }
                attachService.setManuallySelectedProject(manualUrl);
                manualUrl = "";
            }

            // retrieve data
            results = attachService.getSelectedProjects();
            listAdapter =
                    new BatchConflictListAdapter(BatchConflictListActivity.this, R.id.listview, results, getSupportFragmentManager());
            lv.setAdapter(listAdapter);

            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "BatchConflictListActivity setup list with " + results.size() + " elements.");
            }
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
        if(asIsBound) {
            // Detach existing connection.
            unbindService(mASConnection);
            asIsBound = false;
        }
    }

    @Override
    public void onFinish(ProjectAttachWrapper project, Boolean login, String email, String name, String pwd) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BatchConflictListActivity onFinish of dialog");
        }

        if(asIsBound && !attachService.verifyInput(email, name, pwd)) {
            return;
        }

        new AttachProjectAsyncTask(project, login, email, name, pwd).execute();
    }

    @Override
    public ArrayList<String> getDefaultInput() {
        ArrayList<String> values = new ArrayList<>();
        if(asIsBound) {
            values = attachService.getUserDefaultValues();
        }
        return values;
    }

    private class AttachProjectAsyncTask extends AsyncTask<Void, String, Void> {

        ProjectAttachWrapper project;
        Boolean login;
        String email;
        String name;
        String pwd;

        public AttachProjectAsyncTask(ProjectAttachWrapper project, Boolean login, String email, String name, String pwd) {
            this.project = project;
            this.login = login;
            this.email = email;
            this.name = name;
            this.pwd = pwd;
        }

        @Override
        protected void onPreExecute() {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "AttachProjectAsyncTask: " + project.config.name);
            }
            if(asIsBound) {
                project.result = ProjectAttachWrapper.RESULT_ONGOING;
                // adapt layout to changed state
                listAdapter.notifyDataSetChanged();
            }
            else {
                if(Logging.ERROR) {
                    Log.e(Logging.TAG, "AttachProjectAsyncTask: service not bound, cancel.");
                }
                cancel(false);
            }

            super.onPreExecute();
        }

        @Override
        protected Void doInBackground(Void... arg0) {
            // set input credentials
            attachService.setCredentials(email, name, pwd);

            // try attach
            project.lookupAndAttach(login);

            return null;
        }

        @Override
        protected void onPostExecute(Void result) {

            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "AttachProjectAsyncTask.onPostExecute: finished, result: " + project.result);
            }

            // adapt layout to changed state
            listAdapter.notifyDataSetChanged();
            super.onPostExecute(result);
        }
    }
}
