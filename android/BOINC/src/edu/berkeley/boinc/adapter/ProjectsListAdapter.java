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
import android.text.format.DateUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;
import edu.berkeley.boinc.definitions.CommonDefs;
import edu.berkeley.boinc.ProjectsActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.Project;

public class ProjectsListAdapter extends ArrayAdapter<Project> implements OnItemClickListener {
	
	private ArrayList<Project> entries;
    private Activity activity;
    private ListView listView;

    public static class ViewProject {
    	int entryIndex;
        TextView tvProjectName;
        TextView tvProjectStatus;
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

	public String getProjectURL(int position) {
		return entries.get(position).master_url;
	}

	public String getProjectStatus(int position) {
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
            	translateRPCReason(project.sched_rpc_pending)
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

	private String translateRPCReason(int reason) {
	    switch (reason) {
		    case CommonDefs.RPC_REASON_USER_REQ:
		    	return activity.getResources().getString(R.string.rpcreason_userreq);
		    case CommonDefs.RPC_REASON_NEED_WORK:
		    	return activity.getResources().getString(R.string.rpcreason_needwork);
		    case CommonDefs.RPC_REASON_RESULTS_DUE:
		    	return activity.getResources().getString(R.string.rpcreason_resultsdue);
		    case CommonDefs.RPC_REASON_TRICKLE_UP:
		    	return activity.getResources().getString(R.string.rpcreason_trickleup);
		    case CommonDefs.RPC_REASON_ACCT_MGR_REQ:
		    	return activity.getResources().getString(R.string.rpcreason_acctmgrreq);
		    case CommonDefs.RPC_REASON_INIT:
		    	return activity.getResources().getString(R.string.rpcreason_init);
		    case CommonDefs.RPC_REASON_PROJECT_REQ:
		    	return activity.getResources().getString(R.string.rpcreason_projectreq);
		    default:
		    	return activity.getResources().getString(R.string.rpcreason_unknown);
	    }
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
	    	viewProject.tvProjectName = (TextView)vi.findViewById(R.id.project_name);
	    	viewProject.tvProjectStatus = (TextView)vi.findViewById(R.id.project_status);
	    	viewProject.ibProjectUpdate = (ImageButton)vi.findViewById(R.id.project_update);
	    	viewProject.ibProjectDelete = (ImageButton)vi.findViewById(R.id.project_delete);
	    
	        vi.setTag(viewProject);
	        
	    } else {
	    	
	    	viewProject = (ViewProject)vi.getTag();
	    	
	    }

		// Populate UI Elements
	    viewProject.entryIndex = position;
	    viewProject.tvProjectName.setText(getProject(position));
	    viewProject.tvProjectStatus.setText(getProjectStatus(position));
	    if (listView.isItemChecked(position)) {
	    	viewProject.ibProjectUpdate.setVisibility(View.VISIBLE);
	    	viewProject.ibProjectUpdate.setTag(viewProject);
	    	viewProject.ibProjectUpdate.setClickable(true);
	    	viewProject.ibProjectUpdate.setOnClickListener(new OnClickListener() {
	            public void onClick(View v) {
	            	ViewProject viewProject = (ViewProject)v.getTag();
	            	ProjectsActivity a = (ProjectsActivity)activity;
	            	
	            	a.onProjectUpdate(getProject(viewProject.entryIndex), getProjectURL(viewProject.entryIndex));
	            }
	        });
	    		    		    	
	    	viewProject.ibProjectDelete.setVisibility(View.VISIBLE);
	    	viewProject.ibProjectDelete.setTag(viewProject);
	    	viewProject.ibProjectDelete.setClickable(true);
	    	viewProject.ibProjectDelete.setOnClickListener(new OnClickListener() {
	            public void onClick(View v) {
	            	ViewProject viewProject = (ViewProject)v.getTag();
	            	ProjectsActivity a = (ProjectsActivity)activity;
	            	
	            	a.onProjectDelete(getProject(viewProject.entryIndex), getProjectURL(viewProject.entryIndex));
	            }
	        });
	    } else {
	    	viewProject.ibProjectUpdate.setVisibility(View.INVISIBLE);	    	
	    	viewProject.ibProjectUpdate.setClickable(false);
	    	viewProject.ibProjectUpdate.setOnClickListener(null);
	    	
	    	viewProject.ibProjectDelete.setVisibility(View.INVISIBLE);
	    	viewProject.ibProjectDelete.setClickable(false);
	    	viewProject.ibProjectDelete.setOnClickListener(null);
	    }

        return vi;
    }
    
    public void onItemClick(AdapterView<?> adapter, View view, int position, long id ) {
		notifyDataSetChanged();
    }

}
