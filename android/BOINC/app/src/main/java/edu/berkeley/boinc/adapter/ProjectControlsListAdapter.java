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
package edu.berkeley.boinc.adapter;

import java.util.ArrayList;

import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import edu.berkeley.boinc.ProjectsFragment.ProjectControl;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.RpcClient;

public class ProjectControlsListAdapter extends ArrayAdapter<ProjectControl> {
    //private final String TAG = "ProjectControlsListAdapter";

    private ArrayList<ProjectControl> entries; // ID of control texts in strings.xml
    private Activity activity;

    public ProjectControlsListAdapter(Activity activity, ListView listView, int layoutId, ArrayList<ProjectControl> entries) {
        super(activity, layoutId, entries);
        this.entries = entries;
        this.activity = activity;

        listView.setAdapter(this);
    }

    @Override
    public int getCount() {
        return entries.size();
    }

    @Override
    public ProjectControl getItem(int position) {
        return entries.get(position);
    }

    @Override
    public long getItemId(int position) {
        return entries.get(position).operation;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ProjectControl data = entries.get(position);

        View vi =
                ((LayoutInflater) activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.projects_controls_listitem_layout, null);

        TextView tvText = vi.findViewById(R.id.text);
        String text = "";

        switch(data.operation) {
            case RpcClient.PROJECT_UPDATE:
                text = activity.getResources().getString(R.string.projects_control_update);
                break;
            case RpcClient.PROJECT_SUSPEND:
                text = activity.getResources().getString(R.string.projects_control_suspend);
                break;
            case RpcClient.PROJECT_RESUME:
                text = activity.getResources().getString(R.string.projects_control_resume);
                break;
            case RpcClient.PROJECT_ANW:
                text = activity.getResources().getString(R.string.projects_control_allownewtasks);
                break;
            case RpcClient.PROJECT_NNW:
                text = activity.getResources().getString(R.string.projects_control_nonewtasks);
                break;
            case RpcClient.PROJECT_RESET:
                text = activity.getResources().getString(R.string.projects_control_reset);
                break;
            case RpcClient.PROJECT_DETACH:
                tvText.setBackground(activity.getResources().getDrawable(R.drawable.shape_light_red_background));
                text = activity.getResources().getString(R.string.projects_control_remove);
                break;
            case RpcClient.MGR_SYNC:
                text = activity.getResources().getString(R.string.projects_control_sync_acctmgr);
                break;
            case RpcClient.MGR_DETACH:
                tvText.setBackground(activity.getResources().getDrawable(R.drawable.shape_light_red_background));
                text = activity.getResources().getString(R.string.projects_control_remove_acctmgr);
                break;
            case RpcClient.TRANSFER_RETRY:
                text = activity.getResources().getString(R.string.trans_control_retry);
                break;
            case ProjectControl.VISIT_WEBSITE:
                text = activity.getResources().getString(R.string.projects_control_visit_website);
                break;
        }

        //set onclicklistener for expansion
        vi.setOnClickListener(entries.get(position).projectCommandClickListener);

        // set data of standard elements
        tvText.setText(text);

        return vi;
    }
}
