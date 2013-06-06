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


public class GuiLogListAdapter extends ArrayAdapter<String> implements OnItemClickListener {
	
	private ArrayList<String> entries;
    private Activity activity;
    private ListView listView;
    
    public static class ViewEventLog {
    	int entryIndex;
    	CheckBox cbCheck;
        TextView tvMessage;
    }
 
    public GuiLogListAdapter(Activity activity, ListView listView, int textViewResourceId, ArrayList<String> entries) {
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

	@Override
	public String getItem(int position) {
		return entries.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	@Override
    public View getView(int position, View convertView, ViewGroup parent) {
	    View vi = convertView;
		ViewEventLog viewEventLog;
		
		// Only inflate a new view if the ListView does not already have a view assigned.
	    if (convertView == null) {
	    	
	    	vi = ((LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.eventlog_gui_listitem_layout, null);

	        viewEventLog = new ViewEventLog();
	        viewEventLog.cbCheck = (CheckBox)vi.findViewById(R.id.msgs_gui_check);
	        viewEventLog.tvMessage = (TextView)vi.findViewById(R.id.msgs_gui_message);
	    
	        vi.setTag(viewEventLog);
	    } else {
	    	viewEventLog = (ViewEventLog)vi.getTag();
	    }

		// Populate UI Elements
	    viewEventLog.entryIndex = position;
	    viewEventLog.cbCheck.setChecked(listView.isItemChecked(position));
	    viewEventLog.tvMessage.setText(getItem(position));

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
