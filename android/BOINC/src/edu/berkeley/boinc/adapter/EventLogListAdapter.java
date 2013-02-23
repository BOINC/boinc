/*******************************************************************************
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
 ******************************************************************************/
package edu.berkeley.boinc.adapter;

import java.util.ArrayList;
import java.util.Date;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.Message;

import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;
import android.widget.ListView;

public class EventLogListAdapter extends ArrayAdapter<Message> {
	
	private ArrayList<Message> entries;
    private Activity activity;
    private ListView listView;
 
    public EventLogListAdapter(Activity activity, ListView listView, int textViewResourceId, ArrayList<Message> entries) {
        super(activity, textViewResourceId, entries);
        this.entries = entries;
        this.activity = activity;
        this.listView = listView;
    }
 
	@Override
	public int getCount() {
		return entries.size();
	}

	public String getDate(int position) {
		return new Date(entries.get(position).timestamp*1000).toString();
	}

	@Override
	public Message getItem(int position) {
		return entries.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	public String getMessage(int position) {
		return entries.get(position).body;
	}

	public String getProject(int position) {
		return entries.get(position).project;
	}

	@Override
    public View getView(int position, View convertView, ViewGroup parent) {
    	
		// Only inflate a new view if the ListView does not already have a view assigned.
	    if (convertView == null) {
	        LayoutInflater vi = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
	        convertView = vi.inflate(R.layout.eventlog_layout_listitem, null);
	    }

        // Find the layout elements
	    TextView tvMessage = (TextView)convertView.findViewById(R.id.msgs_message);
		TextView tvDate = (TextView)convertView.findViewById(R.id.msgs_date);
		TextView tvProjectName = (TextView)convertView.findViewById(R.id.msgs_project);

		// Populate UI Elements
		tvMessage.setText(getMessage(position));
		tvDate.setText(getDate(position));
		tvProjectName.setText(getProject(position));

        return convertView;

    }
}
