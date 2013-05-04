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
import java.util.Calendar;
import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.text.format.DateUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import edu.berkeley.boinc.ProjectsActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.utils.BOINCUtils;

public class ProjectsListAdapter extends ArrayAdapter<Project> implements OnItemClickListener {
    //private final String TAG = "ProjectsListAdapter";
	
	private ArrayList<Project> entries;
    private Activity activity;
    private ListView listView;

    public static class ViewProject {
    	int entryIndex;
        TextView tvName;
        TextView tvStatus;
        ImageButton ibUpdate;
        ImageButton ibMore;
        ImageView ivIcon;
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

	public String getName(int position) {
		return entries.get(position).project_name;
	}

	public String getURL(int position) {
		return entries.get(position).master_url;
	}
	
	public Bitmap getIcon(int position) {
		return Monitor.getClientStatus().getProjectIcon(entries.get(position).master_url);
	}

	public String getStatus(int position) {
		Project project = getItem(position);
		StringBuffer sb = new StringBuffer();
		
        if (project.suspended_via_gui) {
        	appendToStatus(sb, activity.getResources().getString(R.string.projects_status_suspendedviagui));
        }
        if (project.dont_request_more_work) {
        	appendToStatus(sb, activity.getResources().getString(R.string.projects_status_dontrequestmorework));
        }
        if (project.ended) {
        	appendToStatus(sb, activity.getResources().getString(R.string.projects_status_ended));
        }
        if (project.detach_when_done) {
        	appendToStatus(sb, activity.getResources().getString(R.string.projects_status_detachwhendone));
        }
        if (project.sched_rpc_pending > 0) {
        	appendToStatus(sb, activity.getResources().getString(R.string.projects_status_schedrpcpending));
            appendToStatus(sb,
            	BOINCUtils.translateRPCReason(activity, project.sched_rpc_pending)
            );
        }
        if (project.scheduler_rpc_in_progress) {
        	appendToStatus(sb, activity.getResources().getString(R.string.projects_status_schedrpcinprogress));
        }
        if (project.trickle_up_pending) {
        	appendToStatus(sb, activity.getResources().getString(R.string.projects_status_trickleuppending));
        }
        
        Calendar minRPCTime = Calendar.getInstance();
        Calendar now = Calendar.getInstance();
        minRPCTime.setTimeInMillis((long)project.min_rpc_time*1000);
        if (minRPCTime.compareTo(now) > 0) {
            appendToStatus(
            	sb,
            	activity.getResources().getString(R.string.projects_status_backoff) + " " +
            	DateUtils.formatElapsedTime((minRPCTime.getTimeInMillis() - now.getTimeInMillis()) / 1000)
            );
        }
		
		return sb.toString();
	}

	private void appendToStatus(StringBuffer existing, String additional) {
	    if (existing.length() == 0) {
	        existing.append(additional);
	    } else {
	        existing.append(", ");
	        existing.append(additional);
	    }
	}

	
	@Override
    public View getView(int position, View convertView, ViewGroup parent) {
	    View vi = convertView;
	    ViewProject viewProject;
    	
		// Only inflate a new view if the ListView does not already have a view assigned.
	    if (convertView == null) {
	    	
	    	vi = ((LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.projects_layout_listitem, null);

	    	viewProject = new ViewProject();
	    	viewProject.tvName = (TextView)vi.findViewById(R.id.project_name);
	    	viewProject.tvStatus = (TextView)vi.findViewById(R.id.project_status);
	    	viewProject.ibUpdate = (ImageButton)vi.findViewById(R.id.project_update);
	    	viewProject.ibMore = (ImageButton)vi.findViewById(R.id.project_more);
	    	viewProject.ivIcon = (ImageView)vi.findViewById(R.id.projectIcon);
	    
	        vi.setTag(viewProject);
	        
	    } else {
	    	
	    	viewProject = (ViewProject)vi.getTag();
	    	
	    }

		// Populate UI Elements
	    viewProject.entryIndex = position;
	    viewProject.tvName.setText(getName(position));
	    String statusText = getStatus(position);
	    if(statusText.isEmpty()) {
	    	viewProject.tvStatus.setVisibility(View.GONE);
	    } else {
	    	viewProject.tvStatus.setVisibility(View.VISIBLE);
		    viewProject.tvStatus.setText(statusText);
	    }
	    Bitmap icon = getIcon(position);
	    // if available set icon, if not boinc logo
	    if (icon == null) viewProject.ivIcon.setImageDrawable(getContext().getResources().getDrawable(R.drawable.boinc));
	    else viewProject.ivIcon.setImageBitmap(icon);
	    
	    if (listView.isItemChecked(position)) {
	    	viewProject.ibUpdate.setVisibility(View.VISIBLE);
	    	viewProject.ibUpdate.setTag(viewProject);
	    	viewProject.ibUpdate.setClickable(true);
	    	viewProject.ibUpdate.setOnClickListener(new OnClickListener() {
	            public void onClick(View v) {
	            	ViewProject viewProject = (ViewProject)v.getTag();
	            	ProjectsActivity a = (ProjectsActivity)activity;
	            	
	            	a.onProjectUpdate(getURL(viewProject.entryIndex), getName(viewProject.entryIndex));
	            }
	        });
	    		    		    	
	    	viewProject.ibMore.setVisibility(View.VISIBLE);
	    	viewProject.ibMore.setTag(viewProject);
	    	viewProject.ibMore.setClickable(true);
	    	viewProject.ibMore.setOnClickListener(new OnClickListener() {
	            public void onClick(View v) {
	            	ViewProject viewProject = (ViewProject)v.getTag();
	            	ProjectsActivity a = (ProjectsActivity)activity;
	            	
	            	a.onProjectMore(getURL(viewProject.entryIndex), getName(viewProject.entryIndex));
	            }
	        });
	    } else { // item is not checked
	    	viewProject.ibUpdate.setVisibility(View.INVISIBLE);	    	
	    	viewProject.ibUpdate.setClickable(false);
	    	viewProject.ibUpdate.setOnClickListener(null);
	    	
	    	viewProject.ibMore.setVisibility(View.INVISIBLE);
	    	viewProject.ibMore.setClickable(false);
	    	viewProject.ibMore.setOnClickListener(null);
	    }

        return vi;
    }
    
    public void onItemClick(AdapterView<?> adapter, View view, int position, long id ) {
	    ViewProject viewProject = (ViewProject)view.getTag();
    	((ProjectsActivity)activity).onProjectClicked(getURL(viewProject.entryIndex), getName(viewProject.entryIndex));
		notifyDataSetChanged();
    }

}
