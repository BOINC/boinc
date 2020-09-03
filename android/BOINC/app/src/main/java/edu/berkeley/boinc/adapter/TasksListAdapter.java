/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2016 University of California
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
 */
package edu.berkeley.boinc.adapter;

import android.app.Activity;
import android.graphics.Bitmap;
import android.text.format.DateUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.content.ContextCompat;

import org.apache.commons.lang3.StringUtils;

import java.text.NumberFormat;
import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.time.format.FormatStyle;
import java.util.List;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.TasksFragment.TaskData;
import edu.berkeley.boinc.databinding.TasksLayoutListItemBinding;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.utils.BOINCDefs;
import edu.berkeley.boinc.utils.Logging;

public class TasksListAdapter extends ArrayAdapter<TaskData> {
    private final DateTimeFormatter dateTimeFormatter = DateTimeFormatter.ofLocalizedDateTime(FormatStyle.MEDIUM);
    private final List<TaskData> entries;
    private final Activity activity;
    /**
     * This member eliminates reallocation of a {@link StringBuilder} object in {@link #getView(int, View, ViewGroup)}.
     *
     * @see #getView(int, View, ViewGroup)
     */
    private final StringBuilder elapsedTimeStringBuilder;
    private final NumberFormat percentNumberFormat;

    public TasksListAdapter(Activity activity, int textViewResourceId, List<TaskData> entries) {
        super(activity, textViewResourceId, entries);
        this.entries = entries;
        this.activity = activity;
        this.elapsedTimeStringBuilder = new StringBuilder();
        percentNumberFormat = NumberFormat.getPercentInstance();
        percentNumberFormat.setMinimumFractionDigits(3);
    }

    @NonNull
    @Override
    public View getView(int position, @Nullable View convertView, @NonNull ViewGroup parent) {
        TaskData listItem = entries.get(position);

        // setup new view, if:
        // - view is null, has not been here before
        // - view has different id
        final TasksLayoutListItemBinding binding;
        if (convertView != null && listItem.id.equals(convertView.getTag())) {
            binding = TasksLayoutListItemBinding.bind(convertView);
        } else {
            binding = TasksLayoutListItemBinding.inflate(LayoutInflater.from(parent.getContext()));
        }

        // --- set up view elements that are independent of "active" and "expanded" state
        String finalIconId = (String) binding.projectIcon.getTag();
        if(!StringUtils.equals(finalIconId, listItem.id)) {
            Bitmap icon = getIcon(position);
            // if available set icon, if not boinc logo
            if(icon == null) {
                binding.projectIcon.setImageResource(R.drawable.ic_boinc);
            }
            else {
                binding.projectIcon.setImageBitmap(icon);
                binding.projectIcon.setTag(listItem.id);
            }
        }

        binding.taskHeader.setText(listItem.getResult().getApp().getDisplayName());

        // set project name
        String tempProjectName = listItem.getResult().getProjectURL();
        if(listItem.getResult().getProject() != null) {
            tempProjectName = listItem.getResult().getProject().getName();
            if(listItem.getResult().isProjectSuspendedViaGUI()) {
                tempProjectName = tempProjectName + " " + getContext().getString(R.string.tasks_header_project_paused);
            }
        }
        binding.projectName.setText(tempProjectName);

        // status text
        String statusT = determineStatusText(listItem);
        binding.taskStatus.setText(statusT);
        if(listItem.getResult().getState() == BOINCDefs.RESULT_ABORTED ||
           listItem.getResult().getState() == BOINCDefs.RESULT_COMPUTE_ERROR ||
           listItem.getResult().getState() == BOINCDefs.RESULT_FILES_DOWNLOADING ||
           listItem.getResult().getState() == BOINCDefs.RESULT_FILES_UPLOADED ||
           listItem.getResult().getState() == BOINCDefs.RESULT_FILES_UPLOADING ||
           listItem.getResult().getState() == BOINCDefs.RESULT_READY_TO_REPORT ||
           listItem.getResult().getState() == BOINCDefs.RESULT_UPLOAD_FAILED) {
            binding.taskStatusPercentage.setVisibility(View.GONE);
        }
        else {
            binding.taskStatusPercentage.setVisibility(View.VISIBLE);
            binding.taskStatusPercentage.setText(percentNumberFormat.format(listItem.getResult().getFractionDone()));
        }
        // --- end of independent view elements

        // progress bar: show when task active or expanded
        // result and process state are overlapping, e.g. PROCESS_EXECUTING and RESULT_FILES_DOWNLOADING
        // therefore check also whether task is active
        final boolean active = (listItem.isTaskActive() && listItem.determineState() == BOINCDefs.PROCESS_EXECUTING);
        if(active || listItem.expanded) {
            binding.progressBar.setVisibility(View.VISIBLE);
            binding.progressBar.setIndeterminate(false);
            binding.progressBar.setProgressDrawable(ContextCompat.getDrawable(activity, R.drawable.progressbar));
            binding.progressBar.setProgress(Math.round(listItem.getResult().getFractionDone() * binding.progressBar.getMax()));
        }
        else {
            binding.progressBar.setVisibility(View.GONE);
        }

        // expansion
        if(!listItem.expanded) {
            // view is collapsed
            binding.expandCollapse.setImageResource(R.drawable.ic_baseline_keyboard_arrow_right);
            binding.rightColumnExpandWrapper.setVisibility(View.GONE);
            binding.centerColumnExpandWrapper.setVisibility(View.GONE);
        }
        else {
            // view is expanded
            binding.expandCollapse.setImageResource(R.drawable.ic_baseline_keyboard_arrow_down);
            binding.rightColumnExpandWrapper.setVisibility(View.VISIBLE);
            binding.centerColumnExpandWrapper.setVisibility(View.VISIBLE);

            // elapsed time
            final long elapsedTime;
            // show time depending whether task is active or not
            if(listItem.getResult().isActiveTask()) {
                elapsedTime = (long) listItem.getResult().getElapsedTime(); //is 0 when task finished
            }
            else {
                elapsedTime = (long) listItem.getResult().getFinalElapsedTime();
            }
            binding.taskTime.setText(DateUtils.formatElapsedTime(this.elapsedTimeStringBuilder, elapsedTime));

            // set deadline
            final LocalDateTime deadlineDateTime = LocalDateTime.ofInstant(Instant.ofEpochSecond(
                    listItem.getResult().getReportDeadline()), ZoneId.systemDefault());
            final String deadline = dateTimeFormatter.format(deadlineDateTime);
            binding.deadline.setText(deadline);
            // set application friendly name
            if(listItem.getResult().getApp() != null) {
                binding.taskName.setText(listItem.getResult().getName());
            }

            // buttons
            if(listItem.determineState() == BOINCDefs.PROCESS_ABORTED) { //dont show buttons for aborted task
                binding.rightColumnExpandWrapper.setVisibility(View.INVISIBLE);
            }
            else {
                if(listItem.nextState == -1) { // not waiting for new state
                    binding.suspendResumeTask.setOnClickListener(listItem.iconClickListener);

                    binding.abortTask.setOnClickListener(listItem.iconClickListener);
                    binding.abortTask.setTag(RpcClient.RESULT_ABORT); // tag on button specified operation triggered in iconClickListener
                    binding.abortTask.setVisibility(View.VISIBLE);

                    binding.requestProgressBar.setVisibility(View.GONE);

                    // checking what suspendResume button should be shown
                    if(listItem.getResult().isSuspendedViaGUI()) { // show play
                        binding.suspendResumeTask.setVisibility(View.VISIBLE);
                        binding.suspendResumeTask.setBackgroundColor(ContextCompat.getColor(activity, R.color.dark_green));
                        binding.suspendResumeTask.setImageResource(R.drawable.ic_baseline_play_arrow_white);
                        binding.suspendResumeTask.setTag(RpcClient.RESULT_RESUME); // tag on button specified operation triggered in iconClickListener
                    }
                    else if(listItem.determineState() == BOINCDefs.PROCESS_EXECUTING) { // show pause
                        binding.suspendResumeTask.setVisibility(View.VISIBLE);
                        binding.suspendResumeTask.setBackgroundColor(ContextCompat.getColor(activity, R.color.dark_green));
                        binding.suspendResumeTask.setImageResource(R.drawable.ic_baseline_pause_white);
                        binding.suspendResumeTask.setTag(RpcClient.RESULT_SUSPEND); // tag on button specified operation triggered in iconClickListener
                    }
                    else { // show nothing
                        binding.suspendResumeTask.setVisibility(View.GONE);
                    }
                }
                else {
                    // waiting for a new state
                    binding.suspendResumeTask.setVisibility(View.INVISIBLE);
                    binding.abortTask.setVisibility(View.INVISIBLE);
                    binding.requestProgressBar.setVisibility(View.VISIBLE);
                }
            }
        }

        return binding.getRoot();
    }

    @Nullable
    private Bitmap getIcon(int position) {
        // try to get current client status from monitor
        try {
            return BOINCActivity.monitor.getProjectIcon(entries.get(position).getResult().getProjectURL());
        }
        catch(Exception e) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG, "TasksListAdapter: Could not load data, clientStatus not initialized.");
            }
            return null;
        }
    }

    private String determineStatusText(TaskData tmp) {
        //read status
        int status = tmp.determineState();

        // custom state
        if(status == BOINCDefs.RESULT_SUSPENDED_VIA_GUI) {
            return activity.getString(R.string.tasks_custom_suspended_via_gui);
        }
        if(status == BOINCDefs.RESULT_PROJECT_SUSPENDED) {
            return activity.getString(R.string.tasks_custom_project_suspended_via_gui);
        }
        if(status == BOINCDefs.RESULT_READY_TO_REPORT) {
            return activity.getString(R.string.tasks_custom_ready_to_report);
        }

        //active state
        if(tmp.getResult().isActiveTask()) {
            switch(status) {
                case BOINCDefs.PROCESS_UNINITIALIZED:
                    return activity.getString(R.string.tasks_active_uninitialized);
                case BOINCDefs.PROCESS_EXECUTING:
                    return activity.getString(R.string.tasks_active_executing);
                case BOINCDefs.PROCESS_ABORT_PENDING:
                    return activity.getString(R.string.tasks_active_abort_pending);
                case BOINCDefs.PROCESS_QUIT_PENDING:
                    return activity.getString(R.string.tasks_active_quit_pending);
                case BOINCDefs.PROCESS_SUSPENDED:
                    return activity.getString(R.string.tasks_active_suspended);
                default:
                    if(Logging.WARNING) {
                        Log.w(Logging.TAG, "determineStatusText could not map: " + tmp.determineState());
                    }
                    return "";
            }
        }
        else {
            // passive state
            switch(status) {
                case BOINCDefs.RESULT_NEW:
                    return activity.getString(R.string.tasks_result_new);
                case BOINCDefs.RESULT_FILES_DOWNLOADING:
                    return activity.getString(R.string.tasks_result_files_downloading);
                case BOINCDefs.RESULT_FILES_DOWNLOADED:
                    return activity.getString(R.string.tasks_result_files_downloaded);
                case BOINCDefs.RESULT_COMPUTE_ERROR:
                    return activity.getString(R.string.tasks_result_compute_error);
                case BOINCDefs.RESULT_FILES_UPLOADING:
                    return activity.getString(R.string.tasks_result_files_uploading);
                case BOINCDefs.RESULT_FILES_UPLOADED:
                    return activity.getString(R.string.tasks_result_files_uploaded);
                case BOINCDefs.RESULT_ABORTED:
                    return activity.getString(R.string.tasks_result_aborted);
                case BOINCDefs.RESULT_UPLOAD_FAILED:
                    return activity.getString(R.string.tasks_result_upload_failed);
                default:
                    if(Logging.WARNING) {
                        Log.w(Logging.TAG, "determineStatusText could not map: " + tmp.determineState());
                    }
                    return "";
            }
        }
    }
}
