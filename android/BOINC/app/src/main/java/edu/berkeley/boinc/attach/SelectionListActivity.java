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
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.View;
import android.widget.ListView;
import android.widget.Toast;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.client.IMonitor;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.ProjectInfo;
import edu.berkeley.boinc.utils.Logging;

import java.text.Collator;
import java.util.ArrayList;
import java.util.Collections;

public class SelectionListActivity extends FragmentActivity {

    private ListView lv;
    ArrayList<ProjectListEntry> entries = new ArrayList<>();
    ArrayList<ProjectInfo> selected = new ArrayList<>();

    // services
    private IMonitor monitor = null;
    private boolean mIsBound = false;
    private ProjectAttachService attachService = null;
    private boolean asIsBound = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "AttachProjectListActivity onCreate");
        }

        doBindService();

        // setup layout
        setContentView(R.layout.attach_project_list_layout);
        lv = findViewById(R.id.listview);
    }

    @Override
    protected void onDestroy() {
        if(Logging.VERBOSE) {
            Log.v(Logging.TAG, "AttachProjectListActivity onDestroy");
        }
        doUnbindService();
        super.onDestroy();
    }

    // check whether user has checked at least a single project
    // shows toast otherwise
    private Boolean checkProjectChecked() {
        for(ProjectListEntry tmp : entries) {
            if(tmp.checked) {
                return true;
            }
        }
        Toast toast = Toast.makeText(getApplicationContext(), R.string.attachproject_list_header, Toast.LENGTH_SHORT);
        toast.show();
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "AttachProjectListActivity no project selected, stop!");
        }
        return false;
    }

    // check whether device is online before starting connection attempt
    // as needed for AttachProjectLoginActivity (retrieval of ProjectConfig)
    // note: available internet does not guarantee connection to project server
    // is possible!
    private Boolean checkDeviceOnline() {
        ConnectivityManager connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
        Boolean online = activeNetworkInfo != null && activeNetworkInfo.isConnectedOrConnecting();
        if(!online) {
            Toast toast =
                    Toast.makeText(getApplicationContext(), R.string.attachproject_list_no_internet, Toast.LENGTH_SHORT);
            toast.show();
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "AttachProjectListActivity not online, stop!");
            }
        }
        return online;
    }

    // triggered by continue button
    public void continueClicked(View v) {
        if(!checkProjectChecked()) {
            return;
        }
        if(!checkDeviceOnline()) {
            return;
        }

        StringBuilder selectedProjectsDebug = new StringBuilder();
        // get selected projects
        selected.clear();
        for(ProjectListEntry tmp : entries) {
            if(tmp.checked) {
                selected.add(tmp.info);
                selectedProjectsDebug.append(tmp.info.name).append(",");
            }
        }
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "SelectionListActivity: selected projects: " + selectedProjectsDebug.toString());
        }

        attachService.setSelectedProjects(selected); // returns immediately

        // start credential input activity
        startActivity(new Intent(this, CredentialInputActivity.class));
    }

    private void onCancel() {
        // go to projects screen and clear history
        Intent intent = new Intent(this, BOINCActivity.class);
        // add flags to return to main activity and clearing all others and clear the back stack
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra("targetFragment", R.string.tab_projects); // make activity display projects fragment
        startActivity(intent);
    }

    @Override
    public void onBackPressed() {
        onCancel();
    }

    // triggered by cancel button
    public void cancelClicked(View v) {
        onCancel();
    }

    private ServiceConnection mMonitorConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            monitor = IMonitor.Stub.asInterface(service);
            mIsBound = true;

            UpdateProjectListAsyncTask task = new UpdateProjectListAsyncTask();
            task.execute();
        }

        public void onServiceDisconnected(ComponentName className) {
            // This should not happen
            monitor = null;
            mIsBound = false;
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
        startService(new Intent(this, Monitor.class));
        // Establish a connection with the service, onServiceConnected gets called when
        bindService(new Intent(this, Monitor.class), mMonitorConnection, Service.BIND_AUTO_CREATE);
        // bind to attach service
        bindService(new Intent(this, ProjectAttachService.class), mASConnection, Service.BIND_AUTO_CREATE);
    }

    private void doUnbindService() {
        if(mIsBound) {
            // Detach existing connection.
            unbindService(mMonitorConnection);
            mIsBound = false;
        }
        if(asIsBound) {
            // Detach existing connection.
            unbindService(mASConnection);
            asIsBound = false;
        }
    }

    private class UpdateProjectListAsyncTask extends AsyncTask<Void, Void, ArrayList<ProjectInfo>> {
        @Override
        protected ArrayList<ProjectInfo> doInBackground(Void... arg0) {
            ArrayList<ProjectInfo> data = null;
            boolean retry = true;
            // Try to get the project list for as long as the AsyncTask has not been canceled
            while(retry) {
                try {
                    data = (ArrayList<ProjectInfo>) monitor.getAttachableProjects();
                }
                catch(RemoteException e) {
                    if(Log.isLoggable(Logging.TAG, Log.WARN)) {
                        Log.w(Logging.TAG, e);
                    }
                }
                if(super.isCancelled()) {
                    return data; // Does not matter if data == null or not
                }
                if(data == null) {
                    if(Logging.WARNING) {
                        Log.w(Logging.TAG, "UpdateProjectListAsyncTask: failed to retrieve data, retry....");
                    }
                    try {
                        Thread.sleep(500);
                    }
                    catch(InterruptedException e) {
                        if(Log.isLoggable(Logging.TAG, Log.DEBUG)) {
                            Log.d(Logging.TAG, e.getLocalizedMessage(), e);
                        }
                    }
                }
                else {
                    retry = false;
                }
            }
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "monitor.getAttachableProjects returned with " + data.size() + " elements");
            }
            // Clear current ProjectListEntries since we successfully have got new ProjectInfos
            SelectionListActivity.this.entries.clear();
            // Transform ProjectInfos into ProjectListEntries
            for(int i = data.size() - 1; i >= 0; i--) {
                if(super.isCancelled()) {
                    return data;
                }
                SelectionListActivity.this.entries.add(new ProjectListEntry(data.get(i)));
            }
            // Set preferred Collator for ProjectListEntries before sorting
            if(SelectionListActivity.ProjectListEntry.collator == null) {
                SelectionListActivity.ProjectListEntry.collator = SelectionListActivity.ProjectListEntry.getCollator();
            }
            // Sort ProjectListEntries off the UI thread
            // Unfortunately, there is no way to stop this sort operation if this AsyncTask gets canceled
            Collections.sort(SelectionListActivity.this.entries);
            // Dispose Collator instance after sorting
            SelectionListActivity.ProjectListEntry.collator = null;
            return data;
        }

        protected final void onPostExecute(final ArrayList<ProjectInfo> result) {
            if(result == null) {
                return;
            }

            SelectionListActivity.this.entries.add(new ProjectListEntry()); // add account manager option to bottom of list
            SelectionListAdapter listAdapter =
                    new SelectionListAdapter(SelectionListActivity.this, R.id.listview, entries);
            lv.setAdapter(listAdapter);
        }
    }

    static final class ProjectListEntry implements Comparable<SelectionListActivity.ProjectListEntry> {
        public ProjectInfo info;
        public boolean checked;
        public boolean am; //indicates that element is account manager entry

        /**
         * The {@link Collator} used when comparing {@code ProjectListEntry}s.
         * This member is usually only set when performing comparison operations
         * in bulk. Otherwise, it should be set to {@code null} to avoid having
         * a {@link Collator} instance lingering around when not attaching
         * projects (most of the time). Furthermore, when comparing
         * {@code ProjectListEntry}s in bulk the {@link Collator} instance
         * stored by this member does not need to get reallocated and setup on
         * every comparison.
         *
         * @see SelectionListActivity.ProjectListEntry#getCollator()
         * @see Collator
         */
        static Collator collator;

        public ProjectListEntry(ProjectInfo info) {
            this.info = info;
            this.checked = false;
        }

        /**
         * Creates Account manager list object
         */
        public ProjectListEntry() {
            this.am = true;
        }

        /**
         * Compares this {@code ProjectListEntry} instance to {@code p} based
         * on {@link ProjectInfo#name}. The comparison is <i>case-insensitive</i>.
         *
         * @param p the {@code ProjectListEntry} to compare to
         * @return {@code 0} if both {@link ProjectInfo#name}s are equal,<br>
         * {@code -1} if {@code this} {@link ProjectInfo#name} comes before
         * {@code p}'s {@link ProjectInfo#name} in the current locale's
         * {@link Collator#getInstance() collation},<br>else {@code 1}
         * @see SelectionListActivity.ProjectListEntry#info
         * @see ProjectInfo#name
         * @see Comparable
         */
        public final int compareTo(final SelectionListActivity.ProjectListEntry p) {
            return (SelectionListActivity.ProjectListEntry.collator == null ?
                    SelectionListActivity.ProjectListEntry.getCollator() :
                    SelectionListActivity.ProjectListEntry.collator).compare(this.info.name, p.info.name);
        }

        /**
         * Gets the preferred locale specific {@link Collator} for comparing {@code ProjectListEntry}s.
         *
         * @return the preferred {@link Collator}
         */
        static final Collator getCollator() {
            final Collator collator;
            (collator = Collator.getInstance()).setStrength(Collator.SECONDARY);
            collator.setDecomposition(Collator.NO_DECOMPOSITION);
            return collator;
        }
    }
}
