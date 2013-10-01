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

import edu.berkeley.boinc.utils.*;
import java.util.ArrayList;
import org.apache.http.impl.cookie.DateUtils;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Notice;
import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.text.Html;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class NoticesListAdapter extends ArrayAdapter<Notice>{
	private ArrayList<Notice> entries;
	private Activity activity;

	public NoticesListAdapter(Activity a, int textViewResourceId, ArrayList<Notice> entries) {
		super(a, textViewResourceId, entries);
		this.entries = entries;
		this.activity = a;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {

		Notice listItem = entries.get(position);
		
		LayoutInflater vi = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View v = vi.inflate(R.layout.notices_layout_listitem, null);
		
		ImageView ivIcon = (ImageView)v.findViewById(R.id.projectIcon);
		Bitmap icon = getIcon(position);
		// if available set icon, if not boinc logo
		if(icon == null) { 
			ivIcon.setImageDrawable(getContext().getResources().getDrawable(R.drawable.boinc));
		} else {
			ivIcon.setImageBitmap(icon);
		}
		
		TextView tvProjectName = (TextView) v.findViewById(R.id.projectName);
		tvProjectName.setText(listItem.project_name);
		
		TextView tvNoticeTitle = (TextView) v.findViewById(R.id.noticeTitle);
		tvNoticeTitle.setText(listItem.title);
		
		TextView tvNoticeContent = (TextView) v.findViewById(R.id.noticeContent);
		tvNoticeContent.setText(Html.fromHtml(listItem.description));
		
		TextView tvNoticeTime = (TextView) v.findViewById(R.id.noticeTime);
		tvNoticeTime.setText(DateUtils.formatDate(new java.util.Date((long)listItem.create_time*1000)));
		
		// set tag for onClic
		if(!listItem.link.isEmpty()) v.setTag(listItem.link);

		return v;
	}
	
	private Bitmap getIcon(int position) {
		// try to get current client status from monitor
		ClientStatus status;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"TasksListAdapter: Could not load data, clientStatus not initialized.");
			return null;
		}
		return status.getProjectIconByName(entries.get(position).project_name);
	}

}
