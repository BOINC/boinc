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
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import edu.berkeley.boinc.ProjectsActivity.ProjectsListData;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.utils.BOINCUtils;
import edu.berkeley.boinc.utils.Logging;

public class ProjectsListAdapter extends ArrayAdapter<ProjectsListData> {
    //private final String TAG = "ProjectsListAdapter";
	
	private ArrayList<ProjectsListData> entries;
    private Activity activity;
    
    public ProjectsListAdapter(Activity activity, ListView listView, int textViewResourceId, ArrayList<ProjectsListData> entries) {
        super(activity, textViewResourceId, entries);
        this.entries = entries;
        this.activity = activity;
        
        listView.setAdapter(this);
    }
 
	@Override
	public int getCount() {
		return entries.size();
	}

	@Override
	public ProjectsListData getItem(int position) {
		return entries.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	public String getName(int position) {
		return entries.get(position).project.project_name;
	}

	public String getUser(int position) {
		String user = entries.get(position).project.user_name;
		String team = entries.get(position).project.team_name;
		String userString = user;
		if(!team.isEmpty()) user = user + " (" + team + ")";
		return userString;
	}
	
	public Boolean getIsAcctMgr(int position) {
		return entries.get(position).isMgr;
	}

	public String getURL(int position) {
		return entries.get(position).id;
	}
	
	public Bitmap getIcon(int position) {
		// try to get current client status from monitor
		ClientStatus status;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"ProjectsListAdapter: Could not load data, clientStatus not initialized.");
			return null;
		}
		return status.getProjectIcon(entries.get(position).id);
	}

	public String getStatus(int position) {
		Project project = getItem(position).project;
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
            appendToStatus(sb, BOINCUtils.translateRPCReason(activity, project.sched_rpc_pending));
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

		ProjectsListData data = entries.get(position);
		Boolean isAcctMgr = data.isMgr;

		View vi = convertView;
		// setup new view, if:
		// - view is null, has not been here before
		// - view has different id
		Boolean setup = false;
		if(vi == null) setup = true;
		else {
			String viewId = (String)vi.getTag();
			if(!data.id.equals(viewId)) setup = true;
		}
		
		if(setup){
	    	// first time getView is called for this element
			if(isAcctMgr) vi = ((LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.projects_layout_listitem_acctmgr, null);
			else vi = ((LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.projects_layout_listitem, null);
	    	//set onclicklistener for expansion
			vi.setOnClickListener(entries.get(position).projectsListClickListener);
			vi.setTag(data.id);
		}
		
		if(isAcctMgr) {
			// element is account manager
			
			// populate name
	        TextView tvName = (TextView)vi.findViewById(R.id.name);
	        tvName.setText(data.acctMgrInfo.acct_mgr_name);
	        
	        // populate url
	        TextView tvUrl = (TextView)vi.findViewById(R.id.url);
	        tvUrl.setText(data.acctMgrInfo.acct_mgr_url);
			
		} else {
			// element is project
			
			// set data of standard elements
	        TextView tvName = (TextView)vi.findViewById(R.id.project_name);
	        tvName.setText(getName(position));
	        
	        TextView tvUser = (TextView)vi.findViewById(R.id.project_user);
	        String userText = getUser(position);
	        if(userText.isEmpty()) tvUser.setVisibility(View.GONE);
	        else {
	        	tvUser.setVisibility(View.VISIBLE);
	        	tvUser.setText(userText);
	        }
	        
		    String statusText = getStatus(position);
	        TextView tvStatus = (TextView)vi.findViewById(R.id.project_status);
		    if(statusText.isEmpty()) tvStatus.setVisibility(View.GONE);
		    else {
		    	tvStatus.setVisibility(View.VISIBLE);
		    	tvStatus.setText(statusText);
		    }
		    
		    ImageView ivIcon = (ImageView)vi.findViewById(R.id.project_icon);
		    String finalIconId = (String)ivIcon.getTag();
		    if(finalIconId == null || !finalIconId.equals(data.id)) {
			    Bitmap icon = getIcon(position);
			    // if available set icon, if not boinc logo
			    if (icon == null) {
			    	// boinc logo
			    	ivIcon.setImageDrawable(getContext().getResources().getDrawable(R.drawable.boinc));
			    } else {
			    	// project icon
			    	ivIcon.setImageBitmap(icon);
			    	// mark as final
			    	ivIcon.setTag(data.id);
			    }
		    }
		    
	    	// credits
	    	Integer totalCredit = Double.valueOf(data.project.user_total_credit).intValue();
	    	Integer hostCredit = Double.valueOf(data.project.host_total_credit).intValue();
	    	String creditsText = vi.getContext().getString(R.string.projects_credits_header) + " " + hostCredit;
			TextView tvCredits = (TextView)vi.findViewById(R.id.project_credits);
	    	if(!hostCredit.equals(totalCredit)) // show host credit only if not like user credit
	    		creditsText += " " + vi.getContext().getString(R.string.projects_credits_host_header) + " "
	    					+ totalCredit + " " + vi.getContext().getString(R.string.projects_credits_user_header);
	    	tvCredits.setText(creditsText);
	    	
	    	// icon background
    		RelativeLayout iconBackground = (RelativeLayout)vi.findViewById(R.id.icon_background);
	    	if(data.project.attached_via_acct_mgr) {
	    		iconBackground.setBackgroundDrawable(activity.getApplicationContext().getResources().getDrawable(R.drawable.shape_light_blue_background_wo_stroke));
	    	} else {
	    		iconBackground.setBackgroundColor(activity.getApplicationContext().getResources().getColor(android.R.color.transparent));
	    	}
		}
		
        return vi;
    }
}
