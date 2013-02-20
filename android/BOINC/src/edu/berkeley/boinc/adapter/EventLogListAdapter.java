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

public class EventLogListAdapter extends ArrayAdapter<Message>{
	
	// private final String TAG = "MessagesListAdapter";

	private ArrayList<Message> entries;
    private Activity activity;
 
    public EventLogListAdapter(Activity a, int textViewResourceId, ArrayList<Message> entries) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
    }
 
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
    	
    	// Setup view
        View v = convertView;
        LayoutInflater vi = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        v = vi.inflate(R.layout.eventlog_layout_listitem, null);

    	// Get Message
    	Message listItem = entries.get(position);
    	
		// Construct output
		String project = listItem.project;
		String date = new Date(listItem.timestamp*1000).toString();
		String message = listItem.body;

        // Instantiate layout elements
		TextView tvProjectName = (TextView) v.findViewById(R.id.msgs_project);
		TextView tvDate = (TextView) v.findViewById(R.id.msgs_date);
		TextView tvMessage = (TextView) v.findViewById(R.id.msgs_message);

		// Populate UI Elements
		tvProjectName.setText(project);
		tvDate.setText(date);
		tvMessage.setText(message);

		// Log.d(TAG, "project name: " + projectNameS + " - account: " + accountS);
        return v;

    }
}
