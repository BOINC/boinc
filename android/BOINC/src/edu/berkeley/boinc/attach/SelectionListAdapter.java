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
package edu.berkeley.boinc.attach;

import java.util.ArrayList;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.attach.SelectionListActivity.ProjectListEntry;
import edu.berkeley.boinc.utils.Logging;
import android.content.Context;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.TextView;

public class SelectionListAdapter extends ArrayAdapter<ProjectListEntry>{

	private ArrayList<ProjectListEntry> entries;
    private FragmentActivity activity;
 
    public SelectionListAdapter(FragmentActivity a, int textViewResourceId, ArrayList<ProjectListEntry> entries) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
    }
 
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
    	
        View v = convertView;
        
        LayoutInflater vi = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        
        final ProjectListEntry listItem = entries.get(position);
		
        v = vi.inflate(R.layout.attach_project_list_layout_listitem, null);
		TextView name = (TextView) v.findViewById(R.id.name);
		name.setText(listItem.info.name);
		TextView description = (TextView) v.findViewById(R.id.description);
		description.setText(listItem.info.generalArea);
		TextView summary = (TextView) v.findViewById(R.id.summary);
		summary.setText(listItem.info.summary);
		CheckBox cb = (CheckBox) v.findViewById(R.id.cb);
		cb.setChecked(listItem.checked);
		cb.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				listItem.checked = !listItem.checked;
			}
		});
		LinearLayout textWrapper = (LinearLayout) v.findViewById(R.id.text_wrapper);
		textWrapper.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				if(Logging.DEBUG) Log.d(Logging.TAG, "SelectionListAdapter: onProjectClick open info for: " + listItem.info.name);

				ProjectInfoFragment dialog = ProjectInfoFragment.newInstance(listItem.info);
				dialog.show(activity.getSupportFragmentManager(), "ProjectInfoFragment");
			}
		});
        return v;
    }
	
	@Override
	public int getCount() {
		return entries.size();
	}
}
