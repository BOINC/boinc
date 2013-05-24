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
import edu.berkeley.boinc.rpc.ProjectInfo;

import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class AttachProjectListAdapter extends ArrayAdapter<ProjectInfo>{
	
	//private final String TAG = "AttachProjectListAdapter";
	private ArrayList<ProjectInfo> entries;
    private Activity activity;
 
    public AttachProjectListAdapter(Activity a, int textViewResourceId, ArrayList<ProjectInfo> entries) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
    }
 
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
    	
        View v = convertView;
        LayoutInflater vi = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        
		ProjectInfo listItem = entries.get(position);
		
        v = vi.inflate(R.layout.attach_project_list_layout_listitem, null);
		TextView name = (TextView) v.findViewById(R.id.name);
		TextView description = (TextView) v.findViewById(R.id.description);
		name.setText(listItem.name);
		description.setText(listItem.generalArea);
		v.setTag(listItem); //add ProjectInfo to view
		
        return v;
    }
}
