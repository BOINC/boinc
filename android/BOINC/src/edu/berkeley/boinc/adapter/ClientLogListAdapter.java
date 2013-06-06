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

import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.ListView;
import android.widget.TextView;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.Message;


public class ClientLogListAdapter extends ArrayAdapter<Message> implements OnItemClickListener {
	
	private ArrayList<Message> entries;
    private Activity activity;
    private ListView listView;
    
    public static class ViewEventLog {
    	int entryIndex;
    	CheckBox cbCheck;
        TextView tvMessage;
        TextView tvDate;
        TextView tvProjectName;
    }
 
    public ClientLogListAdapter(Activity activity, ListView listView, int textViewResourceId, ArrayList<Message> entries) {
        super(activity, textViewResourceId, entries);
        this.entries = entries;
        this.activity = activity;
        this.listView = listView;
        
        listView.setAdapter(this);
        listView.setOnItemClickListener(this);
        listView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
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
	    View vi = convertView;
		ViewEventLog viewEventLog;
    	
		// Only inflate a new view if the ListView does not already have a view assigned.
	    if (convertView == null) {
	    	
	    	vi = ((LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.eventlog_client_listitem_layout, null);

	        viewEventLog = new ViewEventLog();
	        viewEventLog.cbCheck = (CheckBox)vi.findViewById(R.id.msgs_check);
	        viewEventLog.tvMessage = (TextView)vi.findViewById(R.id.msgs_message);
	        viewEventLog.tvDate = (TextView)vi.findViewById(R.id.msgs_date);
	        viewEventLog.tvProjectName = (TextView)vi.findViewById(R.id.msgs_project);
	    
	        vi.setTag(viewEventLog);
	        
	    } else {
	    	
	    	viewEventLog = (ViewEventLog)vi.getTag();
	    	
	    }

		// Populate UI Elements
	    viewEventLog.entryIndex = position;
	    viewEventLog.cbCheck.setChecked(listView.isItemChecked(position));
	    viewEventLog.tvMessage.setText(getMessage(position));
	    viewEventLog.tvDate.setText(getDate(position));
	    if(getProject(position).isEmpty()){
	    	viewEventLog.tvProjectName.setVisibility(View.GONE);
	    } else {
	    	viewEventLog.tvProjectName.setVisibility(View.VISIBLE);
	    	viewEventLog.tvProjectName.setText(getProject(position));
	    }

        return vi;
    }

    public void onItemClick(AdapterView<?> adapter, View view, int position, long id ) {
		ViewEventLog viewEventLog = (ViewEventLog)view.getTag();

		if (viewEventLog.cbCheck.isChecked()) {
			viewEventLog.cbCheck.setChecked(false);
		} else {
			viewEventLog.cbCheck.setChecked(true);
		}

		notifyDataSetChanged();
    }

}
