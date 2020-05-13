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
import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;

import org.apache.commons.lang3.StringUtils;

import java.util.List;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.StorageFragment.StorageListData;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.Logging;

public class StorageListAdapter extends ArrayAdapter<StorageListData> {
    private List<StorageListData> entries;
    private Activity activity;

    public StorageListAdapter(Activity activity, ListView listView, int textViewResourceId, List<StorageListData> entries) {
        super(activity, textViewResourceId, entries);
        this.entries = entries;
        this.activity = activity;

        listView.setAdapter(this);
    }

    @Override
    public int getCount() {
        return entries.size();
    }

    @Override
    public StorageListData getItem(int position) {
        return entries.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    public String getName(int position) {
        return entries.get(position).project.getProjectName();
    }

    private String getSize(int position) {
        double serverSize = entries.get(position).project.getDiskUsage();
        return Double.valueOf(serverSize).toString();
    }

    private Bitmap getIcon(int position) {
        // try to get current client status from monitor
        try {
            return BOINCActivity.monitor.getProjectIcon(entries.get(position).id);
        }
        catch(Exception e) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG, "ProjectsListAdapter: Could not load data, clientStatus not initialized.");
            }
            return null;
        }
    }

    @NonNull
    @Override
    public View getView(int position, View convertView, @NonNull ViewGroup parent) {
        StorageListData data = entries.get(position);

        View vi = convertView;
        // setup new view, if:
        // - view is null, has not been here before
        // - view has different id
        boolean setup = false;
        if(vi == null) {
            setup = true;
        }
        else {
            String viewId = (String) vi.getTag();
            if(!data.id.equals(viewId)) {
                setup = true;
            }
        }

        if(setup) {
            final LayoutInflater layoutInflater = (LayoutInflater) activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            assert layoutInflater != null;
            // first time getView is called for this element
            vi = layoutInflater.inflate(R.layout.storage_layout_listitem, null);
            vi.setTag(data.id);
        }

        // element is project
        // set data of standard elements
        TextView tvName = vi.findViewById(R.id.project_name);
        tvName.setText(getName(position));

        TextView tvSize = vi.findViewById(R.id.project_size);
        String sizeText = getSize(position);
        tvSize.setText(sizeText);

        ImageView ivIcon = vi.findViewById(R.id.project_icon);
        String finalIconId = (String) ivIcon.getTag();
        if(!StringUtils.equals(finalIconId, data.id)) {
            Bitmap icon = getIcon(position);
            // if available set icon, if not boinc logo
            if(icon == null) {
                // BOINC logo
                ivIcon.setImageDrawable(getContext().getResources().getDrawable(R.drawable.boinc));
            }
            else {
                // project icon
                ivIcon.setImageBitmap(icon);
                // mark as final
                ivIcon.setTag(data.id);
            }
        }

        // icon background
        RelativeLayout iconBackground = vi.findViewById(R.id.icon_background);
        if(data.project.getAttachedViaAcctMgr()) {
            iconBackground.setBackground(activity.getApplicationContext().getResources().getDrawable(R.drawable.shape_light_blue_background_wo_stroke));
        }
        else {
            iconBackground.setBackgroundColor(activity.getApplicationContext().getResources().getColor(android.R.color.transparent));
        }

        return vi;
    }
}
