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

import java.sql.Date;
import java.util.ArrayList;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.TasksActivity.TaskData;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.utils.BOINCDefs;

import android.app.Activity;
import android.content.Context;
import android.text.format.DateFormat;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class TasksListAdapter extends ArrayAdapter<TaskData>{
	
	//private final String TAG = "TasksListAdapter";
	private ArrayList<TaskData> entries;
	private Activity activity;
 
	public TasksListAdapter(Activity a, int textViewResourceId, ArrayList<TaskData> entries) {
		super(a, textViewResourceId, entries);
		this.entries = entries;
		this.activity = a;
	}
 
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
    	
		View v = convertView;
		LayoutInflater vi = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		v = vi.inflate(R.layout.tasks_layout_listitem, null);
		v.setOnClickListener(entries.get(position).taskClickListener);

		ProgressBar pb = (ProgressBar) v.findViewById(R.id.progressBar);
		TextView header = (TextView) v.findViewById(R.id.taskHeader);
		TextView status = (TextView) v.findViewById(R.id.taskStatus);
		TextView time = (TextView) v.findViewById(R.id.taskTime);
		TextView progress = (TextView) v.findViewById(R.id.taskProgress);
        
		TaskData listItem = entries.get(position);

		pb.setIndeterminate(false);
		pb.setProgressDrawable(this.activity.getResources().getDrawable((determineProgressBarLayout(listItem))));
		
		//v.setTag(listItem.name);
		String headerT = getContext().getString(R.string.tasks_header_friendly_name) + " " + listItem.result.app.getName();
		header.setText(headerT);
		
		Float fraction =  listItem.result.fraction_done;
		if(!listItem.result.active_task && listItem.result.ready_to_report) { //fraction not available, set it to 100
			fraction = Float.valueOf((float) 1.0);
		}
		pb.setProgress(Math.round(fraction * pb.getMax()));
		String pT = Math.round(fraction * 100) + "%";
		progress.setText(pT);
		
		String statusT = determineStatusText(listItem);
		status.setText(statusT);
		
		int elapsedTime = (int)listItem.result.elapsed_time;
		time.setText(String.format("%02d:%02d:%02d", elapsedTime/3600, (elapsedTime/60)%60, elapsedTime%60));

		RelativeLayout ll = (RelativeLayout) v.findViewById(R.id.expansion);
		if (listItem.expanded) {
			ll.setVisibility(View.VISIBLE);
			// update resume/suspend state (button state)
			// set deadline
			String deadline = (String) DateFormat.format("E d MMM yyyy hh:mm:ss aa", new Date(listItem.result.report_deadline*1000));
			((TextView) v.findViewById(R.id.deadline)).setText(getContext().getString(R.string.tasks_header_deadline) + " " + deadline);
			// set project name
			String tempProjectName = listItem.result.project_url;
			if(listItem.result.project != null) {
				tempProjectName = listItem.result.project.getName();
			}
			((TextView) v.findViewById(R.id.projectName)).setText(getContext().getString(R.string.tasks_header_project_name) + " " + tempProjectName);
			// set application friendly name
			if(listItem.result.app != null) {
				((TextView) v.findViewById(R.id.taskName)).setText(getContext().getString(R.string.tasks_header_name) + " " + listItem.result.name);
			}
			
			if(listItem.determineState() == BOINCDefs.PROCESS_ABORTED) { //dont show buttons for aborted task
				((LinearLayout)v.findViewById(R.id.requestPendingWrapper)).setVisibility(View.GONE);
				((LinearLayout)v.findViewById(R.id.taskButtons)).setVisibility(View.GONE);
			} else {
				
				ImageView suspendResume = (ImageView) v.findViewById(R.id.suspendResumeTask);
				suspendResume.setOnClickListener(listItem.iconClickListener);

				ImageView abortButton = (ImageView) v.findViewById(R.id.abortTask);
				abortButton.setOnClickListener(listItem.iconClickListener);
				abortButton.setTag(RpcClient.RESULT_ABORT); // tag on button specified operation triggered in iconClickListener
			
				if (listItem.nextState == -1) { // not waiting for new state
					((LinearLayout)v.findViewById(R.id.requestPendingWrapper)).setVisibility(View.GONE);
					((LinearLayout)v.findViewById(R.id.taskButtons)).setVisibility(View.VISIBLE);
					if (listItem.determineState() == BOINCDefs.PROCESS_EXECUTING) {
						suspendResume.setImageResource(R.drawable.pausew24);
						suspendResume.setTag(RpcClient.RESULT_SUSPEND); // tag on button specified operation triggered in iconClickListener
					} else {
						suspendResume.setImageResource(R.drawable.playw24);
						suspendResume.setTag(RpcClient.RESULT_RESUME); // tag on button specified operation triggered in iconClickListener
					}
				} else {
					((LinearLayout)v.findViewById(R.id.taskButtons)).setVisibility(View.GONE);
					((LinearLayout)v.findViewById(R.id.requestPendingWrapper)).setVisibility(View.VISIBLE);
				}
			}
		} else {
			ll.setVisibility(View.GONE);
		}

		return v;
	}

	private String determineStatusText(TaskData tmp) {
		String text = "";
		if(tmp.result.active_task) {
			switch (tmp.result.active_task_state) {
			case BOINCDefs.PROCESS_UNINITIALIZED:
				text = activity.getString(R.string.tasks_active_uninitialized);
				break;
			case BOINCDefs.PROCESS_EXECUTING:
				text = activity.getString(R.string.tasks_active_executing);
				break;
			case BOINCDefs.PROCESS_ABORT_PENDING:
				text = activity.getString(R.string.tasks_active_abort_pending);
				break;
			case BOINCDefs.PROCESS_QUIT_PENDING:
				text = activity.getString(R.string.tasks_active_quit_pending);
				break;
			case BOINCDefs.PROCESS_SUSPENDED:
				text = activity.getString(R.string.tasks_active_suspended);
				break;
			}
		} else {
			switch (tmp.result.state) {
			case BOINCDefs.RESULT_NEW:
				text = activity.getString(R.string.tasks_result_new);
				break;
			case BOINCDefs.RESULT_FILES_DOWNLOADING:
				text = activity.getString(R.string.tasks_result_files_downloading);
				break;
			case BOINCDefs.RESULT_FILES_DOWNLOADED:
				text = activity.getString(R.string.tasks_result_files_downloaded);
				break;
			case BOINCDefs.RESULT_COMPUTE_ERROR:
				text = activity.getString(R.string.tasks_result_compute_error);
				break;
			case BOINCDefs.RESULT_FILES_UPLOADING:
				text = activity.getString(R.string.tasks_result_files_uploading);
				break;
			case BOINCDefs.RESULT_FILES_UPLOADED:
				text = activity.getString(R.string.tasks_result_files_uploaded);
				break;
			case BOINCDefs.RESULT_ABORTED:
				text = activity.getString(R.string.tasks_result_aborted);
				break;
			case BOINCDefs.RESULT_UPLOAD_FAILED:
				text = activity.getString(R.string.tasks_result_upload_failed);
				break;
			}
		}
		return text;
	}
    
	private Integer determineProgressBarLayout(TaskData tmp) {
		if(tmp.result.active_task) {
			if(tmp.result.active_task_state == BOINCDefs.PROCESS_EXECUTING) {
				//running
				return R.drawable.progressbar_active;
			} else {
				//suspended - ready to run
				return R.drawable.progressbar_paused;
			}
		} else {
			if((tmp.result.state == BOINCDefs.RESULT_ABORTED) || (tmp.result.state == BOINCDefs.RESULT_UPLOAD_FAILED) || (tmp.result.state == BOINCDefs.RESULT_COMPUTE_ERROR)) { 
				//error
				return R.drawable.progressbar_error;
			} else if (tmp.result.ready_to_report) {
				//finished
				return R.drawable.progressbar_active;
			} else {
				//paused or stopped
				return R.drawable.progressbar_paused;
			}
		}
	}
}
