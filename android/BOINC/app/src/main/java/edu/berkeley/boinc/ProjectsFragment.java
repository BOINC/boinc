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
package edu.berkeley.boinc;

import edu.berkeley.boinc.utils.*;

import java.util.ArrayList;
import java.util.Iterator;

import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.RemoteException;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

import edu.berkeley.boinc.adapter.ProjectControlsListAdapter;
import edu.berkeley.boinc.adapter.ProjectsListAdapter;
import edu.berkeley.boinc.attach.ManualUrlInputFragment;
import edu.berkeley.boinc.rpc.Notice;
import edu.berkeley.boinc.rpc.AcctMgrInfo;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.rpc.Transfer;

public class ProjectsFragment extends Fragment {

    private ListView lv;
    private ProjectsListAdapter listAdapter;
    private ArrayList<ProjectsListData> data = new ArrayList<>();

    // controls popup dialog
    Dialog dialogControls;

    // BroadcastReceiver event is used to update the UI with updated information from
    // the client.  This is generally called once a second.
    //
    private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
    private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            //if(Logging.DEBUG) Log.d(Logging.TAG, "ClientStatusChange - onReceive()");
            populateLayout();
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        setHasOptionsMenu(true); // enables fragment specific menu
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        if(Logging.VERBOSE) {
            Log.v(Logging.TAG, "ProjectsFragment onCreateView");
        }
        // Inflate the layout for this fragment
        View layout = inflater.inflate(R.layout.projects_layout, container, false);
        lv = layout.findViewById(R.id.projectsList);
        listAdapter = new ProjectsListAdapter(getActivity(), lv, R.id.projectsList, data);
        return layout;
    }

    @Override
    public void onPause() {
        if(Logging.VERBOSE) {
            Log.d(Logging.TAG, "ProjectsFragment onPause()");
        }

        getActivity().unregisterReceiver(mClientStatusChangeRec);
        super.onPause();
    }

    @Override
    public void onResume() {
        if(Logging.VERBOSE) {
            Log.d(Logging.TAG, "ProjectsFragment onResume()");
        }
        super.onResume();

        populateLayout();

        getActivity().registerReceiver(mClientStatusChangeRec, ifcsc);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        // appends the project specific menu to the main menu.
        inflater.inflate(R.menu.projects_menu, menu);
        super.onCreateOptionsMenu(menu, inflater);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if(Logging.VERBOSE) {
            Log.v(Logging.TAG, "AttachProjectListActivity onOptionsItemSelected()");
        }

        switch(item.getItemId()) {
            case R.id.projects_add_url:
                ManualUrlInputFragment dialog2 = new ManualUrlInputFragment();
                dialog2.show(getFragmentManager(), getActivity().getString(R.string.attachproject_list_manual_button));
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    private void populateLayout() {
        try {
            // read projects from state saved in ClientStatus
            ArrayList<Project> statusProjects = (ArrayList<Project>) BOINCActivity.monitor.getProjects();
            AcctMgrInfo statusAcctMgr = BOINCActivity.monitor.getClientAcctMgrInfo();
            ArrayList<Transfer> statusTransfers = (ArrayList<Transfer>) BOINCActivity.monitor.getTransfers();

            // get server / scheduler notices to display if device does not meet
            ArrayList<Notice> serverNotices = (ArrayList<Notice>) BOINCActivity.monitor.getServerNotices();

            // Update Project data
            updateData(statusProjects, statusAcctMgr, serverNotices, statusTransfers);

            // Force list adapter to refresh
            listAdapter.notifyDataSetChanged();

        }
        catch(Exception e) {
            // data retrieval failed, set layout to loading...
            if(Logging.ERROR) {
                Log.d(Logging.TAG, "ProjectsActiviy data retrieval failed.");
            }
        }
    }

    private void updateData(ArrayList<Project> latestRpcProjectsList, AcctMgrInfo acctMgrInfo, ArrayList<Notice> serverNotices, ArrayList<Transfer> ongoingTransfers) {

        // ACCOUNT MANAGER
        //loop through list adapter array to find index of account manager entry (0 || 1 manager possible)
        int mgrIndex = -1;
        for(int x = 0; x < data.size(); x++) {
            if(data.get(x).isMgr) {
                mgrIndex = x;
                //break; // This function has to be revised. Are we searching the firs or the last account manager? Are there more or only one possible?
            }
        }
        if(mgrIndex < 0) { // no manager present until now
            if(Logging.VERBOSE) {
                Log.d(Logging.TAG, "No manager found in layout list. New entry available: " + acctMgrInfo.present);
            }
            if(acctMgrInfo.present) {
                // add new manager entry, at top of the list
                data.add(new ProjectsListData(null, acctMgrInfo, null));
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "New acct mgr found: " + acctMgrInfo.acct_mgr_name);
                }
            }
        }
        else { // manager found in existing list
            if(Logging.VERBOSE) {
                Log.d(Logging.TAG, "Manager found in layout list at index: " + mgrIndex);
            }
            if(!acctMgrInfo.present) {
                // manager got detached, remove from list
                data.remove(mgrIndex);
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "Acct mgr removed from list.");
                }
            }
        }

        // ATTACHED PROJECTS
        //loop through all received Result items to add new projects
        for(Project rpcResult : latestRpcProjectsList) {
            //check whether this project is new
            Integer index = null;
            for(int x = 0; x < data.size(); x++) {
                if(rpcResult.master_url.equals(data.get(x).id)) {
                    index = x;
                    //break; // Need more further investigation.
                }
            }
            if(index == null) { // Project is new, add
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "New project found, id: " + rpcResult.master_url + ", managed: " +
                                       rpcResult.attached_via_acct_mgr);
                }
                if(rpcResult.attached_via_acct_mgr) {
                    data.add(new ProjectsListData(rpcResult, null, mapTransfersToProject(rpcResult.master_url, ongoingTransfers))); // append to end of list (after manager)
                }
                else {
                    data.add(0, new ProjectsListData(rpcResult, null, mapTransfersToProject(rpcResult.master_url, ongoingTransfers))); // put at top of list (before manager)
                }
            }
            else { // Project was present before, update its data
                data.get(index).updateProjectData(rpcResult, null, mapTransfersToProject(rpcResult.master_url, ongoingTransfers));
            }
        }

        //loop through the list adapter to find removed (ready/aborted) projects
        // use iterator to safely remove while iterating
        Iterator<ProjectsListData> iData = data.iterator();
        while(iData.hasNext()) {
            Boolean found = false;
            ProjectsListData listItem = iData.next();
            if(listItem.isMgr) {
                continue;
            }
            for(Project rpcResult : latestRpcProjectsList) {
                if(listItem.id.equals(rpcResult.master_url)) {
                    found = true;
                    break;
                }
            }
            if(!found) {
                iData.remove();
            }
        }

        // SERVER NOTICES
        // loop through active projects to add/remove server notices
        if(serverNotices != null) {
            int mappedServerNotices = 0;
            for(ProjectsListData project : data) {
                if(project.isMgr) {
                    continue; // do not seek notices in manager entries (crashes)
                }
                boolean noticeFound = false;
                for(Notice serverNotice : serverNotices) {
                    if(project.project.project_name.equals(serverNotice.project_name)) {
                        project.addServerNotice(serverNotice);
                        noticeFound = true;
                        mappedServerNotices++;
                    }
                }
                if(!noticeFound) {
                    project.addServerNotice(null);
                }
            }
            if(mappedServerNotices != serverNotices.size()) {
                if(Logging.WARNING) {
                    Log.w(Logging.TAG, "could not match notice: " + mappedServerNotices + "/" + serverNotices.size());
                }
            }
        }
    }

    // takes list of all ongoing transfers and a project id (url) and returns transfer that belong to given project
    private ArrayList<Transfer> mapTransfersToProject(String id, ArrayList<Transfer> allTransfers) {
        ArrayList<Transfer> projectTransfers = new ArrayList<>();
        for(Transfer trans : allTransfers) {
            if(trans.project_url.equals(id)) {
                // project id matches url in transfer, add to list
                projectTransfers.add(trans);
            }
        }
        if(Logging.VERBOSE) {
            Log.d(Logging.TAG, "ProjectsActivity mapTransfersToProject() mapped " + projectTransfers.size() +
                               " transfers to project " + id);
        }
        return projectTransfers;
    }

    // data wrapper for list view
    public class ProjectsListData {
        // can be either project or account manager
        public Project project;
        public Notice lastServerNotice = null;
        public AcctMgrInfo acctMgrInfo;
        public ArrayList<Transfer> projectTransfers;
        public String id; // == url
        public boolean isMgr;
        public ProjectsListData listEntry = this;

        public ProjectsListData(Project data, AcctMgrInfo acctMgrInfo, ArrayList<Transfer> projectTransfers) {
            this.project = data;
            this.acctMgrInfo = acctMgrInfo;
            this.projectTransfers = projectTransfers;
            if(this.project == null && this.acctMgrInfo != null) {
                isMgr = true;
            }
            if(isMgr) {
                this.id = acctMgrInfo.acct_mgr_url;
            }
            else {
                this.id = data.master_url;
            }
        }

        public void updateProjectData(Project data, AcctMgrInfo acctMgrInfo, ArrayList<Transfer> projectTransfers) {
            if(isMgr) {
                this.acctMgrInfo = acctMgrInfo;
            }
            else {
                this.project = data;
                this.projectTransfers = projectTransfers;
            }
        }

        public void addServerNotice(Notice notice) {
            this.lastServerNotice = notice;
        }

        public Notice getLastServerNotice() {
            return lastServerNotice;
        }

        // handles onClick on list element, could be either project or account manager
        // sets up dialog with controls
        public final OnClickListener projectsListClickListener = new OnClickListener() {
            @Override
            public void onClick(View v) {
                dialogControls = new Dialog(getActivity());
                // layout
                dialogControls.requestWindowFeature(Window.FEATURE_NO_TITLE);
                dialogControls.setContentView(R.layout.dialog_list);
                ListView list = dialogControls.findViewById(R.id.options);

                // add control items depending on:
                // - type, account manager vs. project
                // - client status, e.g. either suspend or resume
                // - show advanced preference
                // - project attached via account manager (e.g. hide Remove)
                ArrayList<ProjectControl> controls = new ArrayList<>();
                if(isMgr) {
                    ((TextView) dialogControls.findViewById(R.id.title)).setText(R.string.projects_control_dialog_title_acctmgr);

                    controls.add(new ProjectControl(listEntry, ProjectControl.VISIT_WEBSITE));
                    controls.add(new ProjectControl(listEntry, RpcClient.MGR_SYNC));
                    controls.add(new ProjectControl(listEntry, RpcClient.MGR_DETACH));
                }
                else {
                    ((TextView) dialogControls.findViewById(R.id.title)).setText(R.string.projects_control_dialog_title);

                    controls.add(new ProjectControl(listEntry, ProjectControl.VISIT_WEBSITE));
                    if(projectTransfers != null && !projectTransfers.isEmpty()) {
                        controls.add(new ProjectControl(listEntry, RpcClient.TRANSFER_RETRY));
                    }
                    controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_UPDATE));
                    if(project.suspended_via_gui) {
                        controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_RESUME));
                    }
                    else {
                        controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_SUSPEND));
                    }
                    boolean isShowAdvanced;
                    try {
                        isShowAdvanced = BOINCActivity.monitor.getShowAdvanced();
                    }
                    catch(RemoteException e) {
                        isShowAdvanced = false;
                    }
                    if(isShowAdvanced && project.dont_request_more_work) {
                        controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_ANW));
                    }
                    if(isShowAdvanced && !project.dont_request_more_work) {
                        controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_NNW));
                    }
                    if(isShowAdvanced) {
                        controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_RESET));
                    }
                    if(!project.attached_via_acct_mgr) {
                        controls.add(new ProjectControl(listEntry, RpcClient.PROJECT_DETACH));
                    }
                }

                // list adapter
                list.setAdapter(new ProjectControlsListAdapter(getActivity(), list, R.layout.projects_controls_listitem_layout, controls));
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "dialog list adapter entries: " + controls.size());
                }

                // buttons
                Button cancelButton = dialogControls.findViewById(R.id.cancel);
                cancelButton.setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        dialogControls.dismiss();
                    }
                });

                // show dialog
                dialogControls.show();
            }
        };
    }

    public class ProjectControl {
        public ProjectsListData data;
        public Integer operation;

        // operation that do not imply an RPC are defined here
        public static final int VISIT_WEBSITE = 100;

        public ProjectControl(ProjectsListData data, Integer operation) {
            this.operation = operation;
            this.data = data;
        }

        // handles onClick on list element in control dialog
        // might show confirmation dialog depending on operation type
        public final OnClickListener projectCommandClickListener = new OnClickListener() {
            @Override
            public void onClick(View v) {

                //check whether command requires confirmation
                if(operation == RpcClient.PROJECT_DETACH
                   || operation == RpcClient.PROJECT_RESET
                   || operation == RpcClient.MGR_DETACH) {
                    final Dialog dialog = new Dialog(getActivity());
                    dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
                    dialog.setContentView(R.layout.dialog_confirm);
                    Button confirm = dialog.findViewById(R.id.confirm);
                    TextView tvTitle = dialog.findViewById(R.id.title);
                    TextView tvMessage = dialog.findViewById(R.id.message);

                    // operation dependend texts
                    if(operation == RpcClient.PROJECT_DETACH) {
                        tvTitle.setText(R.string.projects_confirm_detach_title);
                        tvMessage.setText(getString(R.string.projects_confirm_detach_message) + " "
                                          + data.project.project_name + " " +
                                          getString(R.string.projects_confirm_detach_message2));
                        confirm.setText(R.string.projects_confirm_detach_confirm);
                    }
                    else if(operation == RpcClient.PROJECT_RESET) {
                        tvTitle.setText(R.string.projects_confirm_reset_title);
                        tvMessage.setText(getString(R.string.projects_confirm_reset_message) + " "
                                          + data.project.project_name +
                                          getString(R.string.projects_confirm_reset_message2));
                        confirm.setText(R.string.projects_confirm_reset_confirm);
                    }
                    else if(operation == RpcClient.MGR_DETACH) {
                        tvTitle.setText(R.string.projects_confirm_remove_acctmgr_title);
                        tvMessage.setText(getString(R.string.projects_confirm_remove_acctmgr_message) + " "
                                          + data.acctMgrInfo.acct_mgr_name +
                                          getString(R.string.projects_confirm_remove_acctmgr_message2));
                        confirm.setText(R.string.projects_confirm_remove_acctmgr_confirm);
                    }

                    confirm.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            new ProjectOperationAsync().execute(data, operation);
                            dialog.dismiss();
                            dialogControls.dismiss();
                        }
                    });
                    Button cancel = dialog.findViewById(R.id.cancel);
                    cancel.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            dialog.dismiss();
                        }
                    });
                    dialog.show();
                }
                else if(operation ==
                        ProjectControl.VISIT_WEBSITE) { // command does not require confirmation and is not rpc based
                    dialogControls.dismiss();
                    Intent i = new Intent(Intent.ACTION_VIEW, Uri.parse(data.id));
                    startActivity(i);
                }
                else { // command does not required confirmation, but is rpc based
                    new ProjectOperationAsync().execute(data, operation);
                    dialogControls.dismiss();
                }
            }
        };
    }

    // executes project operations in new thread
    private final class ProjectOperationAsync extends AsyncTask<Object, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Object... params) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "ProjectOperationAsync doInBackground");
            }
            try {
                ProjectsListData data = (ProjectsListData) params[0];
                Integer operation = (Integer) params[1];
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG,
                          "ProjectOperationAsync isMgr: " + data.isMgr + "url: " + data.id + " operation: " +
                          operation);
                }

                switch(operation) {
                    // project operations
                    case RpcClient.PROJECT_UPDATE:
                    case RpcClient.PROJECT_SUSPEND:
                    case RpcClient.PROJECT_RESUME:
                    case RpcClient.PROJECT_NNW:
                    case RpcClient.PROJECT_ANW:
                    case RpcClient.PROJECT_DETACH:
                    case RpcClient.PROJECT_RESET:
                        return BOINCActivity.monitor.projectOp(operation, data.id);

                    // acct mgr operations
                    case RpcClient.MGR_SYNC:
                        return BOINCActivity.monitor.synchronizeAcctMgr(data.acctMgrInfo.acct_mgr_url);
                    case RpcClient.MGR_DETACH:
                        return BOINCActivity.monitor.addAcctMgrErrorNum("", "", "").code == BOINCErrors.ERR_OK;

                    // transfer operations
                    case RpcClient.TRANSFER_RETRY:
                        return BOINCActivity.monitor.transferOperation(data.projectTransfers, operation);
                    case RpcClient.TRANSFER_ABORT:
                        break;

                    default:
                        if(Logging.ERROR) {
                            Log.e(Logging.TAG, "ProjectOperationAsync could not match operation: " + operation);
                        }
                }
            }
            catch(Exception e) {
                if(Logging.WARNING) {
                    Log.w(Logging.TAG, "ProjectOperationAsync error in do in background", e);
                }
            }
            return false;
        }

        @Override
        protected void onPostExecute(Boolean success) {
            if(success) {
                try {
                    BOINCActivity.monitor.forceRefresh();
                }
                catch(RemoteException e) {
                    e.printStackTrace();
                }
            }
            else if(Logging.WARNING) {
                Log.w(Logging.TAG, "ProjectOperationAsync failed.");
            }
        }
    }
}
