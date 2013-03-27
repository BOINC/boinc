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

import edu.berkeley.boinc.R;

import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class AttachProjectListAdapter extends ArrayAdapter<AttachListItemWrapper>{
	
	//private final String TAG = "AttachProjectListAdapter";
	private ArrayList<AttachListItemWrapper> entries;
    private Activity activity;
 
    public AttachProjectListAdapter(Activity a, int textViewResourceId, ArrayList<AttachListItemWrapper> entries) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
    }
 
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
    	
        View v = convertView;
        LayoutInflater vi = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        
		AttachListItemWrapper listItem = entries.get(position);
		
		if(listItem.isCategory) {
	        v = vi.inflate(R.layout.attach_project_list_layout_listitem_category, null);
			TextView name = (TextView) v.findViewById(R.id.category_header);
			name.setText(listItem.categoryName);
		} else if (listItem.isProject) {
	        v = vi.inflate(R.layout.attach_project_list_layout_listitem, null);
			TextView name = (TextView) v.findViewById(R.id.name);
			TextView description = (TextView) v.findViewById(R.id.description);
			name.setText(listItem.project.name);
			description.setText(listItem.project.generalArea);
			v.setTag(listItem.project); //add ProjectInfo to view
		} else if (listItem.isManual) {
	        v = vi.inflate(R.layout.attach_project_list_layout_listitem_manual, null);
			TextView name = (TextView) v.findViewById(R.id.name);
			name.setText(R.string.attachproject_list_manual_header);
		}
		
    	
        return v;
    }
}
