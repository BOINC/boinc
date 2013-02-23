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
import android.widget.ListView;
import android.widget.TextView;
import android.widget.ImageButton;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;

public class ProjectsListAdapter extends ArrayAdapter<Project> implements OnItemClickListener {
	
	private ArrayList<Project> entries;
    private Activity activity;
    private ListView listView;

    public static class ViewProject {
    	int entryIndex;
        TextView tvProjectName;
        TextView tvUserName;
        ImageButton ibProjectUpdate;
        ImageButton ibProjectDelete;
    }
    
    public ProjectsListAdapter(Activity activity, ListView listView, int textViewResourceId, ArrayList<Project> entries) {
        super(activity, textViewResourceId, entries);
        this.entries = entries;
        this.activity = activity;
        this.listView = listView;
        
        listView.setAdapter(this);
        listView.setOnItemClickListener(this);
        listView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
    }
 
	@Override
	public int getCount() {
		return entries.size();
	}

	@Override
	public Project getItem(int position) {
		return entries.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	public String getProject(int position) {
		return entries.get(position).project_name;
	}

	public String getUserName(int position) {
		return entries.get(position).user_name;
	}

	@Override
    public View getView(int position, View convertView, ViewGroup parent) {
	    View vi = convertView;
	    ViewProject viewProject;
    	
		// Only inflate a new view if the ListView does not already have a view assigned.
	    if (convertView == null) {
	    	
	    	vi = ((LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.projects_layout_listitem, null);

	    	viewProject = new ViewProject();
	    	viewProject.tvProjectName = (TextView)vi.findViewById(R.id.project_name);
	    	viewProject.tvUserName = (TextView)vi.findViewById(R.id.project_username);
	    	viewProject.ibProjectUpdate = (ImageButton)vi.findViewById(R.id.project_update);
	    	viewProject.ibProjectDelete = (ImageButton)vi.findViewById(R.id.project_delete);
	    
	        vi.setTag(viewProject);
	        
	    } else {
	    	
	    	viewProject = (ViewProject)vi.getTag();
	    	
	    }

		// Populate UI Elements
	    viewProject.entryIndex = position;
	    viewProject.tvProjectName.setText(getProject(position));
	    viewProject.tvUserName.setText(getUserName(position));
	    if (listView.isItemChecked(position)) {
	    	viewProject.ibProjectUpdate.setVisibility(View.VISIBLE);	    	
	    	viewProject.ibProjectDelete.setVisibility(View.VISIBLE);	    	
	    } else {
	    	viewProject.ibProjectUpdate.setVisibility(View.INVISIBLE);	    	
	    	viewProject.ibProjectDelete.setVisibility(View.INVISIBLE);	    	
	    }

        return vi;
    }
    
    public void onItemClick(AdapterView<?> adapter, View view, int position, long id ) {
		notifyDataSetChanged();
    }

}
