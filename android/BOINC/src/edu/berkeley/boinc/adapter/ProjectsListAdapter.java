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
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.net.Uri;
import android.text.SpannableString;
import android.text.format.DateUtils;
import android.text.style.UnderlineSpan;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import edu.berkeley.boinc.ProjectsActivity.ProjectData;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.utils.BOINCUtils;

public class ProjectsListAdapter extends ArrayAdapter<ProjectData> {
    private final String TAG = "ProjectsListAdapter";
	
	private ArrayList<ProjectData> entries;
    private Activity activity;
	
    private Integer screenHeight;
    private Integer screenWidth;
    
    public ProjectsListAdapter(Activity activity, ListView listView, int textViewResourceId, ArrayList<ProjectData> entries) {
        super(activity, textViewResourceId, entries);
        this.entries = entries;
        this.activity = activity;
        
        WindowManager wm = (WindowManager) activity.getSystemService(Context.WINDOW_SERVICE);
        Display display = wm.getDefaultDisplay();
		screenWidth = display.getWidth();
		screenHeight = display.getHeight();
		Log.d(TAG,"screen dimensions: " + screenWidth + "*" + screenHeight);
        
        listView.setAdapter(this);
    }
 
	@Override
	public int getCount() {
		return entries.size();
	}

	@Override
	public ProjectData getItem(int position) {
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

	public String getURL(int position) {
		return entries.get(position).id;
	}
	
	public Bitmap getIcon(int position) {
		return Monitor.getClientStatus().getProjectIcon(entries.get(position).id);
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
	    ProjectData data = entries.get(position);
    	vi = ((LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.projects_layout_listitem, null);
    	
    	//set onclicklistener for expansion
		vi.setOnClickListener(entries.get(position).projectClickListener);
	    
	    // set data of standard elements
        TextView tvName = (TextView)vi.findViewById(R.id.project_name);
        tvName.setText(getName(position));
        
        TextView tvUser = (TextView)vi.findViewById(R.id.project_user);
        String userText = getUser(position);
        if(userText.isEmpty()) tvUser.setVisibility(View.GONE);
        else tvUser.setText(userText);
        
	    String statusText = getStatus(position);
        TextView tvStatus = (TextView)vi.findViewById(R.id.project_status);
	    if(statusText.isEmpty()) tvStatus.setVisibility(View.GONE);
	    else tvStatus.setText(statusText);
	    
	    ImageView ivIcon = (ImageView)vi.findViewById(R.id.project_icon);
	    Bitmap icon = getIcon(position);
	    // if available set icon, if not boinc logo
	    if (icon == null) ivIcon.setImageDrawable(getContext().getResources().getDrawable(R.drawable.boinc));
	    else ivIcon.setImageBitmap(icon);
        
        if(data.expanded) { // expansion elements
            LinearLayout llProjectExtensionWrapper = (LinearLayout)vi.findViewById(R.id.project_expansion_wrapper);
        	llProjectExtensionWrapper.setVisibility(View.VISIBLE);
        	
        	// credits
        	Integer totalCredit = Double.valueOf(data.project.user_total_credit).intValue();
        	Integer hostCredit = Double.valueOf(data.project.host_total_credit).intValue();
        	String creditsText = vi.getContext().getString(R.string.projects_credits_header) + " " + hostCredit;
    		TextView tvCredits = (TextView)vi.findViewById(R.id.project_credits);
        	if(!hostCredit.equals(totalCredit)) // show host credit only if not like user credit
        		creditsText += " " + vi.getContext().getString(R.string.projects_credits_host_header) + " "
        					+ totalCredit + " " + vi.getContext().getString(R.string.projects_credits_user_header);
        	tvCredits.setText(creditsText);
        	
        	// website
        	final String website = data.project.master_url;
        	TextView tvWebsite = (TextView)vi.findViewById(R.id.project_website);
        	SpannableString content = new SpannableString(website);
        	content.setSpan(new UnderlineSpan(), 0, content.length(), 0);
        	tvWebsite.setText(content);
        	final Context ctx = getContext();
        	tvWebsite.setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
					Intent i = new Intent(Intent.ACTION_VIEW);
					i.setData(Uri.parse(website));
					ctx.startActivity(i);}});
        	
        	// buttons
        	Boolean advanced = Monitor.getAppPrefs().getShowAdvanced();
        	Button bUpdate = (Button)vi.findViewById(R.id.project_control_update);
        	bUpdate.setTag(RpcClient.PROJECT_UPDATE);
        	bUpdate.setOnClickListener(entries.get(position).iconClickListener);
        	Button bRemove = (Button)vi.findViewById(R.id.project_control_remove);
        	bRemove.setTag(RpcClient.PROJECT_DETACH);
        	bRemove.setOnClickListener(entries.get(position).iconClickListener);
        	if(advanced) { 
            	// show advanced options only if enabled in preferences
	        	Button bAdvanced = (Button)vi.findViewById(R.id.project_control_advanced);
	        	bAdvanced.setTag(RpcClient.PROJECT_ADVANCED);
	        	bAdvanced.setOnClickListener(entries.get(position).iconClickListener);
	        	bAdvanced.setVisibility(View.VISIBLE);
        	}
        	
        	// adapt layout for wide screens
        	float minWidthInPx;
        	if(advanced) minWidthInPx = convertDpToPixel(vi.getResources().getInteger(R.integer.projects_min_screen_width_for_3_parallel_buttons_dp), vi.getContext());
        	else minWidthInPx = convertDpToPixel(vi.getResources().getInteger(R.integer.projects_min_screen_width_for_2_parallel_buttons_dp), vi.getContext());
        	//Log.d(TAG,"screen width: " + screenWidth + " min. width: " + minWidthInPx);
        	if(screenWidth >= minWidthInPx) {
        		LinearLayout buttonWrapper = (LinearLayout) vi.findViewById(R.id.button_wrapper);
        		buttonWrapper.setOrientation(LinearLayout.HORIZONTAL);
        	}
        }
        return vi;
    }
	
	/**
	 * This method converts dp unit to equivalent pixels, depending on device density. 
	 * 
	 * @param dp A value in dp (density independent pixels) unit. Which we need to convert into pixels
	 * @param context Context to get resources and device specific display metrics
	 * @return A float value to represent px equivalent to dp depending on device density
	 */
	public static float convertDpToPixel(int dp, Context context){
	    Resources resources = context.getResources();
	    DisplayMetrics metrics = resources.getDisplayMetrics();
	    float px = dp * (metrics.densityDpi / 160f);
	    return px;
	}
}
