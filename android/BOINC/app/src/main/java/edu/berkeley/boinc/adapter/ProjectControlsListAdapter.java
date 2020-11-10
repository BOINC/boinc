/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2020 University of California
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

import android.app.Activity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.content.res.AppCompatResources;

import java.util.List;

import edu.berkeley.boinc.ProjectsFragment;
import edu.berkeley.boinc.ProjectsFragment.ProjectControl;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.RpcClient;

public class ProjectControlsListAdapter extends ArrayAdapter<ProjectControl> {
    private final List<ProjectControl> entries; // ID of control texts in strings.xml

    public ProjectControlsListAdapter(Activity activity, List<ProjectControl> entries) {
        super(activity, R.layout.projects_controls_listitem_layout, entries);
        this.entries = entries;
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
        return entries.get(position).getOperation();
    }

    @NonNull
    @Override
    public View getView(int position, View convertView, @NonNull ViewGroup parent) {
        ProjectControl data = entries.get(position);

        View vi = LayoutInflater.from(parent.getContext())
                                .inflate(R.layout.projects_controls_listitem_layout, null);

        TextView tvText = vi.findViewById(R.id.text);
        String text = "";

        switch(data.getOperation()) {
            case RpcClient.PROJECT_UPDATE:
                text = getContext().getResources().getString(R.string.projects_control_update);
                break;
            case RpcClient.PROJECT_SUSPEND:
                text = getContext().getResources().getString(R.string.projects_control_suspend);
                break;
            case RpcClient.PROJECT_RESUME:
                text = getContext().getResources().getString(R.string.projects_control_resume);
                break;
            case RpcClient.PROJECT_ANW:
                text = getContext().getResources().getString(R.string.projects_control_allownewtasks);
                break;
            case RpcClient.PROJECT_NNW:
                text = getContext().getResources().getString(R.string.projects_control_nonewtasks);
                break;
            case RpcClient.PROJECT_RESET:
                text = getContext().getResources().getString(R.string.projects_control_reset);
                break;
            case RpcClient.PROJECT_DETACH:
                tvText.setBackground(AppCompatResources.getDrawable(getContext(), R.drawable.shape_light_red_background));
                text = getContext().getResources().getString(R.string.projects_control_remove);
                break;
            case RpcClient.MGR_SYNC:
                text = getContext().getResources().getString(R.string.projects_control_sync_acctmgr);
                break;
            case RpcClient.MGR_DETACH:
                tvText.setBackground(AppCompatResources.getDrawable(getContext(), R.drawable.shape_light_red_background));
                text = getContext().getResources().getString(R.string.projects_control_remove_acctmgr);
                break;
            case RpcClient.TRANSFER_RETRY:
                text = getContext().getResources().getString(R.string.trans_control_retry);
                break;
            case ProjectsFragment.VISIT_WEBSITE:
                text = getContext().getResources().getString(R.string.projects_control_visit_website);
                break;
            default:
                break;
        }

        //set onclicklistener for expansion
        vi.setOnClickListener(entries.get(position).projectCommandClickListener);

        // set data of standard elements
        tvText.setText(text);

        return vi;
    }
}
