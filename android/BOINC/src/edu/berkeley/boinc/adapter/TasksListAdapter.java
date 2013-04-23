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
import edu.berkeley.boinc.rpc.Result;
import edu.berkeley.boinc.utils.BOINCDefs;

import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ProgressBar;
import android.widget.TextView;

public class TasksListAdapter extends ArrayAdapter<Result>{
	
	//private final String TAG = "TasksListAdapter";
	private ArrayList<Result> entries;
    private Activity activity;
 
    public TasksListAdapter(Activity a, int textViewResourceId, ArrayList<Result> entries) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
    }
 
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
    	
        View v = convertView;
        LayoutInflater vi = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        v = vi.inflate(R.layout.tasks_layout_listitem, null);
		ProgressBar pb = (ProgressBar) v.findViewById(R.id.progressBar);
		TextView header = (TextView) v.findViewById(R.id.taskName);
		TextView status = (TextView) v.findViewById(R.id.taskStatus);
		TextView time = (TextView) v.findViewById(R.id.taskTime);
		TextView progress = (TextView) v.findViewById(R.id.taskProgress);
        
    	Result listItem = entries.get(position);
    	
		pb.setIndeterminate(false);
    	pb.setProgressDrawable(this.activity.getResources().getDrawable((determineProgressBarLayout(listItem))));
		
		//v.setTag(listItem.name);
    	String headerT = "Name: " + listItem.name;
		header.setText(headerT);
		
		Float fraction =  listItem.fraction_done;
		if(!listItem.active_task && listItem.ready_to_report) { //fraction not available, set it to 100
			fraction = Float.valueOf((float) 1.0);
		}
		pb.setProgress(Math.round(fraction * pb.getMax()));
		String pT = Math.round(fraction * 100) + "%";
		progress.setText(pT);
		
		String statusT = determineStatusText(listItem);
		status.setText(statusT);
		
		int elapsedTime = (int)listItem.elapsed_time;
		time.setText(String.format("%02d:%02d:%02d", elapsedTime/3600, (elapsedTime/60)%60, elapsedTime%60));
    	
        return v;
    }
    
    private String determineStatusText(Result tmp) {
    	String text = "";
    	if(tmp.active_task) {
    		switch (tmp.active_task_state) {
    		case 0:
    			text = activity.getString(R.string.tasks_active_uninitialized);
    			break;
    		case 1:
    			text = activity.getString(R.string.tasks_active_executing);
    			break;
    		case 5:
    			text = activity.getString(R.string.tasks_active_abort_pending);
    			break;
    		case 8:
    			text = activity.getString(R.string.tasks_active_quit_pending);
    			break;
    		case 9:
    			text = activity.getString(R.string.tasks_active_suspended);
    			break;
    		}
    	} else {
    		switch (tmp.state) {
    		case 0:
    			text = activity.getString(R.string.tasks_result_new);
    			break;
    		case 1:
    			text = activity.getString(R.string.tasks_result_files_downloading);
    			break;
    		case 2:
    			text = activity.getString(R.string.tasks_result_files_downloaded);
    			break;
    		case 3:
    			text = activity.getString(R.string.tasks_result_compute_error);
    			break;
    		case 4:
    			text = activity.getString(R.string.tasks_result_files_uploading);
    			break;
    		case 5:
    			text = activity.getString(R.string.tasks_result_files_uploaded);
    			break;
    		case 6:
    			text = activity.getString(R.string.tasks_result_aborted);
    			break;
    		case 7:
    			text = activity.getString(R.string.tasks_result_upload_failed);
    			break;
    		}
    	}
    	return text;
    }
    
    private Integer determineProgressBarLayout(Result tmp) {
    	if(tmp.active_task) {
    		if(tmp.active_task_state == BOINCDefs.PROCESS_EXECUTING) {
    			//running
    			return R.drawable.progressbar_active;
    		} else {
    			//suspended - ready to run
    			return R.drawable.progressbar_paused;
    		}
    	} else {
    		if((tmp.state == BOINCDefs.RESULT_ABORTED) || (tmp.state == BOINCDefs.RESULT_UPLOAD_FAILED) || (tmp.state == BOINCDefs.RESULT_COMPUTE_ERROR)) { 
    			//error
    			return R.drawable.progressbar_error;
    		} else if (tmp.ready_to_report) {
    			//finished
    			return R.drawable.progressbar_active;
    		} else {
    			//paused or stopped
    			return R.drawable.progressbar_paused;
    		}
    	}
    }
}
