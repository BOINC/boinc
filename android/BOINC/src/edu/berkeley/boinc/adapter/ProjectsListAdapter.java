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
import edu.berkeley.boinc.rpc.Project;

import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class ProjectsListAdapter extends ArrayAdapter<Project>{
	
	//private final String TAG = "ProjectsListAdapter";
	private ArrayList<Project> entries;
    private Activity activity;
 
    public ProjectsListAdapter(Activity a, int textViewResourceId, ArrayList<Project> entries) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
    }
 
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
    	
    	//get file that which shall be presented
    	Project listItem = entries.get(position);
    	
    	//setup view
        View v = convertView;
        LayoutInflater vi = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        v = vi.inflate(R.layout.projects_layout_listitem, null);
        //instanciate layout elements
		TextView projectName = (TextView) v.findViewById(R.id.project_name);
		TextView account = (TextView) v.findViewById(R.id.account);
		//customize layout elements
		String projectNameS = listItem.project_name;
		projectName.setText(projectNameS);
		String accountS = listItem.user_name + " - " + "aquired credits: " + listItem.host_total_credit;
		account.setText(accountS);
		//Log.d(TAG,"project name: " + projectNameS + " - account: " + accountS);
		//set tag to be parsed in confirmation dialog
		v.setTag(listItem); //append project information to view
        return v;
    }
}
