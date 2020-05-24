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
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.text.format.DateUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;
import androidx.core.content.res.ResourcesCompat;

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
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.utils.BOINCDefs;
import edu.berkeley.boinc.utils.Logging;

public class TasksListAdapter extends ArrayAdapter<TaskData> {
    private List<TaskData> entries;
    private Activity activity;
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
    public View getView(int position, View convertView, @NonNull ViewGroup parent) {
        TaskData listItem = entries.get(position);

        View v = convertView;
        // setup new view, if:
        // - view is null, has not been here before
        // - view has different id
        boolean setup = false;
        if(v == null) {
            setup = true;
        }
        else {
            String viewId = (String) v.getTag();
            if(!listItem.id.equals(viewId)) {
                setup = true;
            }
        }

        if(setup) {
            LayoutInflater vi = ContextCompat.getSystemService(activity, LayoutInflater.class);
            assert vi != null;
            v = vi.inflate(R.layout.tasks_layout_listitem, null);
            v.setTag(listItem.id);
        }

        ProgressBar pb = v.findViewById(R.id.progressBar);
        TextView header = v.findViewById(R.id.taskHeader);
        TextView status = v.findViewById(R.id.taskStatus);
        TextView time = v.findViewById(R.id.taskTime);
        TextView statusPercentage = v.findViewById(R.id.taskStatusPercentage);
        ImageView expandButton = v.findViewById(R.id.expandCollapse);

        // --- set up view elements that are independent of "active" and "expanded" state
        ImageView ivIcon = v.findViewById(R.id.projectIcon);
        String finalIconId = (String) ivIcon.getTag();
        if(!StringUtils.equals(finalIconId, listItem.id)) {
            Bitmap icon = getIcon(position);
            // if available set icon, if not boinc logo
            if(icon == null) {
                ivIcon.setImageDrawable(getContext().getResources().getDrawable(R.drawable.ic_boinc));
            }
            else {
                ivIcon.setImageBitmap(icon);
                ivIcon.setTag(listItem.id);
            }
        }

        String headerT = listItem.getResult().getApp().getDisplayName();
        header.setText(headerT);

        // set project name
        String tempProjectName = listItem.getResult().getProjectURL();
        if(listItem.getResult().getProject() != null) {
            tempProjectName = listItem.getResult().getProject().getName();
            if(listItem.getResult().isProjectSuspendedViaGUI()) {
                tempProjectName = tempProjectName + " " + getContext().getString(R.string.tasks_header_project_paused);
            }
        }
        ((TextView) v.findViewById(R.id.projectName)).setText(tempProjectName);

        // status text
        String statusT = determineStatusText(listItem);
        status.setText(statusT);
        if(listItem.getResult().getState() == BOINCDefs.RESULT_ABORTED ||
           listItem.getResult().getState() == BOINCDefs.RESULT_COMPUTE_ERROR ||
           listItem.getResult().getState() == BOINCDefs.RESULT_FILES_DOWNLOADING ||
           listItem.getResult().getState() == BOINCDefs.RESULT_FILES_UPLOADED ||
           listItem.getResult().getState() == BOINCDefs.RESULT_FILES_UPLOADING ||
           listItem.getResult().getState() == BOINCDefs.RESULT_READY_TO_REPORT ||
           listItem.getResult().getState() == BOINCDefs.RESULT_UPLOAD_FAILED) {
            statusPercentage.setVisibility(View.GONE);
        }
        else {
            statusPercentage.setVisibility(View.VISIBLE);
            statusPercentage.setText(this.percentNumberFormat.format(listItem.getResult().getFractionDone()));
        }
        // --- end of independent view elements

        // progress bar: show when task active or expanded
        // result and process state are overlapping, e.g. PROCESS_EXECUTING and RESULT_FILES_DOWNLOADING
        // therefore check also whether task is active
        final boolean active = (listItem.isTaskActive() && listItem.determineState() == BOINCDefs.PROCESS_EXECUTING);
        if(active || listItem.expanded) {
            pb.setVisibility(View.VISIBLE);
            pb.setIndeterminate(false);
            pb.setProgressDrawable(this.activity.getResources().getDrawable(R.drawable.progressbar));
            pb.setProgress(Math.round(listItem.getResult().getFractionDone() * pb.getMax()));
        }
        else {
            pb.setVisibility(View.GONE);
        }

        // expansion
        RelativeLayout rightColumnExpandWrapper = v.findViewById(R.id.rightColumnExpandWrapper);
        LinearLayout centerColumnExpandWrapper = v.findViewById(R.id.centerColumnExpandWrapper);
        if(!listItem.expanded) {
            // view is collapsed
            expandButton.setImageResource(R.drawable.ic_baseline_keyboard_arrow_right_blue);
            rightColumnExpandWrapper.setVisibility(View.GONE);
            centerColumnExpandWrapper.setVisibility(View.GONE);
        }
        else {
            // view is expanded
            expandButton.setImageResource(R.drawable.ic_baseline_keyboard_arrow_down_blue);
            rightColumnExpandWrapper.setVisibility(View.VISIBLE);
            centerColumnExpandWrapper.setVisibility(View.VISIBLE);

            // elapsed time
            final long elapsedTime;
            // show time depending whether task is active or not
            if(listItem.getResult().isActiveTask()) {
                elapsedTime = (long) listItem.getResult().getElapsedTime(); //is 0 when task finished
            }
            else {
                elapsedTime = (long) listItem.getResult().getFinalElapsedTime();
            }
            time.setText(DateUtils.formatElapsedTime(this.elapsedTimeStringBuilder, elapsedTime));

            // set deadline
            final LocalDateTime deadlineDateTime = LocalDateTime.ofInstant(Instant.ofEpochSecond(
                    listItem.getResult().getReportDeadline()), ZoneId.systemDefault());
            final String deadline = DateTimeFormatter.ofLocalizedDateTime(FormatStyle.MEDIUM)
                                                     .format(deadlineDateTime);
            ((TextView) v.findViewById(R.id.deadline)).setText(deadline);
            // set application friendly name
            if(listItem.getResult().getApp() != null) {
                ((TextView) v.findViewById(R.id.taskName)).setText(listItem.getResult().getName());
            }

            // buttons
            ImageView suspendResume = v.findViewById(R.id.suspendResumeTask);
            ImageView abortButton = v.findViewById(R.id.abortTask);
            if(listItem.determineState() == BOINCDefs.PROCESS_ABORTED) { //dont show buttons for aborted task
                rightColumnExpandWrapper.setVisibility(View.INVISIBLE);
            }
            else {
                if(listItem.nextState == -1) { // not waiting for new state
                    suspendResume.setOnClickListener(listItem.iconClickListener);

                    abortButton.setOnClickListener(listItem.iconClickListener);
                    abortButton.setTag(RpcClient.RESULT_ABORT); // tag on button specified operation triggered in iconClickListener
                    abortButton.setVisibility(View.VISIBLE);

                    (v.findViewById(R.id.request_progressBar)).setVisibility(View.GONE);

                    final Resources resources = activity.getResources();
                    final Resources.Theme theme = activity.getTheme();

                    // checking what suspendResume button should be shown
                    if(listItem.getResult().isSuspendedViaGUI()) { // show play
                        suspendResume.setVisibility(View.VISIBLE);
                        suspendResume.setBackgroundColor(ResourcesCompat.getColor(resources, R.color.dark_green,
                                                                                  theme));
                        suspendResume.setImageResource(R.drawable.ic_baseline_play_arrow_white);
                        suspendResume.setTag(RpcClient.RESULT_RESUME); // tag on button specified operation triggered in iconClickListener
                    }
                    else if(listItem.determineState() == BOINCDefs.PROCESS_EXECUTING) { // show pause
                        suspendResume.setVisibility(View.VISIBLE);
                        suspendResume.setBackgroundColor(ResourcesCompat.getColor(resources, R.color.dark_green,
                                                                                  theme));
                        suspendResume.setImageResource(R.drawable.ic_baseline_pause_white);
                        suspendResume.setTag(RpcClient.RESULT_SUSPEND); // tag on button specified operation triggered in iconClickListener
                    }
                    else { // show nothing
                        suspendResume.setVisibility(View.GONE);
                    }
                }
                else {
                    // waiting for a new state
                    suspendResume.setVisibility(View.INVISIBLE);
                    abortButton.setVisibility(View.INVISIBLE);
                    (v.findViewById(R.id.request_progressBar)).setVisibility(View.VISIBLE);
                }
            }
        }

        return v;
    }

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
