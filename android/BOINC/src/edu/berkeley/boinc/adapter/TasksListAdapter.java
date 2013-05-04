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
import edu.berkeley.boinc.utils.BOINCDefs;

import android.app.Activity;
import android.content.Context;
import android.text.format.DateFormat;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
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
		TextView header = (TextView) v.findViewById(R.id.taskName);
		TextView status = (TextView) v.findViewById(R.id.taskStatus);
		TextView time = (TextView) v.findViewById(R.id.taskTime);
		TextView progress = (TextView) v.findViewById(R.id.taskProgress);
        
		TaskData listItem = entries.get(position);

		pb.setIndeterminate(false);
		pb.setProgressDrawable(this.activity.getResources().getDrawable((determineProgressBarLayout(listItem))));
		
		//v.setTag(listItem.name);
		String headerT = "Name: " + listItem.result.name;
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
			listItem.currentRunState = listItem.determineRunningState();
			// update abort state (button state)
			listItem.currentAbortState = listItem.determineAbortState();
			// set deadline
			String deadline = (String) DateFormat.format("E d MMM yyyy hh:mm:ss aa", new Date(listItem.result.report_deadline*1000));
			((TextView) v.findViewById(R.id.deadline)).setText("Deadline: " + deadline);
			// set project name
			String tempProjectName = listItem.result.project_url;
			if(listItem.result.project != null) {
				tempProjectName = listItem.result.project.getName();
			}
			((TextView) v.findViewById(R.id.projectName)).setText("Project name: " + tempProjectName);
			// set application friendly name
			if(listItem.result.app != null) {
				((TextView) v.findViewById(R.id.friendlyAppName)).setText("App Name: " + listItem.result.app.getName());
			}

			ImageView suspendResume = (ImageView) v.findViewById(R.id.suspendResumeTask);
			if (listItem.currentRunState == listItem.nextRunState) {
				// current button state is same as expected
				suspendResume.setEnabled(true);
				suspendResume.setClickable(true);
				if (listItem.currentRunState == TaskData.TASK_STATE_RUNNING) {
					suspendResume.setOnClickListener(listItem.suspendClickListener);
					suspendResume.setImageResource(R.drawable.pausew24);
				} else {
					suspendResume.setOnClickListener(listItem.resumeClickListener);
					suspendResume.setImageResource(R.drawable.playw24);
				}
			} else {
				// waiting for transient state to be as expected
				// button needs to be disabled (no action should be taken while waiting)
				suspendResume.setEnabled(false);
				suspendResume.setClickable(false);
				suspendResume.setOnClickListener(null);
				if (listItem.currentRunState == TaskData.TASK_STATE_RUNNING) {
					suspendResume.setImageResource(R.drawable.pausew24);
				} else {
					suspendResume.setImageResource(R.drawable.playw24);
				}
			}
			if (listItem.currentAbortState == listItem.nextAbortState) {
				// current button state is same as expected
				if (listItem.currentAbortState == TaskData.TASK_STATE_ABORTED) {
					suspendResume.setEnabled(false);
					suspendResume.setClickable(false);
				} else { 
					suspendResume.setEnabled(true);
					suspendResume.setClickable(true);
					((ImageView) v.findViewById(R.id.abortTask)).setOnClickListener(listItem.abortClickListener);
				}
			} else {
				// waiting for transient state to be as expected
				// button needs to be disabled (no action should be taken while waiting)
				suspendResume.setEnabled(false);
				suspendResume.setClickable(false);
				suspendResume.setOnClickListener(null);
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
			switch (tmp.result.state) {
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
