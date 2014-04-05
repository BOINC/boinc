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
import edu.berkeley.boinc.attach.ProjectAttachService.ProjectAttachWrapper;
import edu.berkeley.boinc.utils.Logging;
import android.app.Activity;
import android.content.Context;
import android.support.v4.app.FragmentManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

public class BatchConflictListAdapter extends ArrayAdapter<ProjectAttachWrapper>{

	private ArrayList<ProjectAttachWrapper> entries;
    private Activity activity;
    private FragmentManager fmgr;
 
    public BatchConflictListAdapter(Activity a, int textViewResourceId, ArrayList<ProjectAttachWrapper> entries, FragmentManager fm) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
        this.fmgr = fm;
    }
 
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {

        final ProjectAttachWrapper listItem = entries.get(position);
        
        View v = convertView;
        if(v == null || ((Integer)v.getTag()) != listItem.result) {
	        LayoutInflater vi = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
	
	        if(Logging.DEBUG) Log.d(Logging.TAG, "BatchConflictListAdapter.getView update for: " + listItem.name + " attach status: " + listItem.result);
			
	        v = vi.inflate(R.layout.attach_project_batch_conflicts_listitem, null);
			TextView name = (TextView) v.findViewById(R.id.name);
			name.setText(listItem.name);
			TextView status = (TextView) v.findViewById(R.id.status);
			Button resolveButton = (Button) v.findViewById(R.id.resolve_button);
			ImageView statusImage = (ImageView) v.findViewById(R.id.status_image);
			ProgressBar statusPb = (ProgressBar) v.findViewById(R.id.status_pb);
			if(listItem.result == ProjectAttachWrapper.RESULT_SUCCESS) {
				// success
				status.setVisibility(View.GONE);
				resolveButton.setVisibility(View.GONE);
				statusPb.setVisibility(View.GONE);
				statusImage.setVisibility(View.VISIBLE);
				statusImage.setImageDrawable(activity.getResources().getDrawable(R.drawable.checkb));
			} else if(listItem.result == ProjectAttachWrapper.RESULT_ONGOING || listItem.result == ProjectAttachWrapper.RESULT_UNINITIALIZED){
				// ongoing
				status.setVisibility(View.GONE);
				resolveButton.setVisibility(View.GONE);
				statusImage.setVisibility(View.GONE);
				statusPb.setVisibility(View.VISIBLE);
			} else if(listItem.result == ProjectAttachWrapper.RESULT_READY) {
				// ready
				statusPb.setVisibility(View.GONE);
				status.setVisibility(View.VISIBLE);
				status.setText(listItem.getResultDescription());
				resolveButton.setVisibility(View.VISIBLE);
				resolveButton.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						if(Logging.DEBUG) Log.d(Logging.TAG, "BatchConflictListAdapter: start resolution dialog for: " + listItem.name);
						IndividualCredentialInputFragment dialog = IndividualCredentialInputFragment.newInstance(listItem);
						dialog.show(fmgr, listItem.name);
					}
				});
				statusImage.setVisibility(View.INVISIBLE);
			} else if(listItem.result == ProjectAttachWrapper.RESULT_CONFIG_DOWNLOAD_FAILED) {
				// download failed, can not continue from here.
				// if user wants to retry, need to go back to selection activity
				statusPb.setVisibility(View.GONE);
				status.setVisibility(View.VISIBLE);
				status.setText(listItem.getResultDescription());
				resolveButton.setVisibility(View.GONE);
			} else {
				// failed
				statusPb.setVisibility(View.GONE);
				status.setVisibility(View.VISIBLE);
				status.setText(listItem.getResultDescription());
				resolveButton.setVisibility(View.VISIBLE);
				resolveButton.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {
						if(Logging.DEBUG) Log.d(Logging.TAG, "BatchConflictListAdapter: start resolution dialog for: " + listItem.name);
						IndividualCredentialInputFragment dialog = IndividualCredentialInputFragment.newInstance(listItem);
						dialog.show(fmgr, listItem.name);
					}
				});
				statusImage.setVisibility(View.VISIBLE);
				statusImage.setImageDrawable(activity.getResources().getDrawable(R.drawable.failedb));
			}
			v.setTag(listItem.result);
        }
        return v;
    }
	
	@Override
	public int getCount() {
		return entries.size();
	}
}
