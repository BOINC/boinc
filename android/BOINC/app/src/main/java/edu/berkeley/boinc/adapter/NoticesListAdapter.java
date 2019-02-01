/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2016 University of California
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
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.text.Html;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.Notice;
import edu.berkeley.boinc.utils.Logging;

import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Date;

public class NoticesListAdapter extends ArrayAdapter<Notice> {
    private ArrayList<Notice> entries;
    private Activity activity;

    public NoticesListAdapter(Activity a, int textViewResourceId, ArrayList<Notice> entries) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {

        final Notice listItem = entries.get(position);

        LayoutInflater vi = (LayoutInflater) activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View v = vi.inflate(R.layout.notices_layout_listitem, null);

        ImageView ivIcon = v.findViewById(R.id.projectIcon);
        Bitmap icon = getIcon(position);
        // if available set icon, if not boinc logo
        if(icon == null) {
            ivIcon.setImageDrawable(getContext().getResources().getDrawable(R.drawable.boinc));
        }
        else {
            ivIcon.setImageBitmap(icon);
        }

        TextView tvProjectName = v.findViewById(R.id.projectName);
        tvProjectName.setText(listItem.project_name);

        TextView tvNoticeTitle = v.findViewById(R.id.noticeTitle);
        tvNoticeTitle.setText(listItem.title);

        TextView tvNoticeContent = v.findViewById(R.id.noticeContent);
        tvNoticeContent.setText(Html.fromHtml(listItem.description));

        TextView tvNoticeTime = v.findViewById(R.id.noticeTime);
        tvNoticeTime.setText(DateFormat.getDateTimeInstance(DateFormat.LONG, DateFormat.SHORT).format(new Date(
                (long) listItem.create_time * 1000)));

        v.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "noticeClick: " + listItem.link);
                }

                if(listItem.link != null && !listItem.link.isEmpty()) {
                    Intent i = new Intent(Intent.ACTION_VIEW, Uri.parse(listItem.link));
                    activity.startActivity(i);
                }

            }
        });

        return v;
    }

    private Bitmap getIcon(int position) {
        // try to get current client status from monitor
        //ClientStatus status;
        try {
            //status  = Monitor.getClientStatus();
            return BOINCActivity.monitor.getProjectIconByName(entries.get(position).project_name);
        }
        catch(Exception e) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG, "TasksListAdapter: Could not load data, clientStatus not initialized.");
            }
            return null;
        }
        //return status.getProjectIconByName(entries.get(position).project_name);
    }

}
