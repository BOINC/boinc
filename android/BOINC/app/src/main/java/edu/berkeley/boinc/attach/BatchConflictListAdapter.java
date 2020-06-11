/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2020 University of California
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
package edu.berkeley.boinc.attach;

import android.app.Activity;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import androidx.annotation.DrawableRes;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.FragmentManager;

import java.util.List;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.attach.ProjectAttachService.ProjectAttachWrapper;
import edu.berkeley.boinc.utils.Logging;

public class BatchConflictListAdapter extends ArrayAdapter<ProjectAttachWrapper> {
    private List<ProjectAttachWrapper> entries;
    private Activity activity;
    private FragmentManager fmgr;

    BatchConflictListAdapter(Activity a, int textViewResourceId, List<ProjectAttachWrapper> entries, FragmentManager fm) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
        this.fmgr = fm;
    }

    @NonNull
    @Override
    public View getView(int position, View convertView, @NonNull ViewGroup parent) {
        final ProjectAttachWrapper listItem = entries.get(position);

        if(Logging.VERBOSE) {
            Log.d(Logging.TAG, "BatchConflictListAdapter.getView for: " + listItem.name + " at position: " + position +
                               " with result: " + listItem.result);
        }

        LayoutInflater vi = ContextCompat.getSystemService(activity, LayoutInflater.class);
        assert vi != null;
        View v = vi.inflate(R.layout.attach_project_batch_conflicts_listitem, null);
        TextView name = v.findViewById(R.id.name);
        name.setText(listItem.name);
        TextView status = v.findViewById(R.id.status);
        ImageView resolveIv = v.findViewById(R.id.resolve_button_image);
        ImageView statusImage = v.findViewById(R.id.status_image);
        ProgressBar statusPb = v.findViewById(R.id.status_pb);
        RelativeLayout itemWrapper = v.findViewById(R.id.resolve_item_wrapper);
        if(listItem.result == ProjectAttachWrapper.RESULT_SUCCESS) {
            // success
            status.setVisibility(View.GONE);
            resolveIv.setVisibility(View.GONE);
            statusPb.setVisibility(View.GONE);
            statusImage.setVisibility(View.VISIBLE);
            statusImage.setImageDrawable(getThemedDrawable(R.drawable.ic_baseline_check,
                                                           R.drawable.ic_baseline_check_white));
        }
        else if(listItem.result == ProjectAttachWrapper.RESULT_ONGOING ||
                listItem.result == ProjectAttachWrapper.RESULT_UNINITIALIZED) {
            // ongoing
            status.setVisibility(View.GONE);
            resolveIv.setVisibility(View.GONE);
            statusImage.setVisibility(View.GONE);
            statusPb.setVisibility(View.VISIBLE);
        }
        else if(listItem.result == ProjectAttachWrapper.RESULT_READY) {
            // ready
            status.setVisibility(View.VISIBLE);
            status.setText(listItem.getResultDescription());
            resolveIv.setVisibility(View.VISIBLE);
            itemWrapper.setOnClickListener(view -> {
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "BatchConflictListAdapter: start resolution dialog for: " + listItem.name);
                }
                IndividualCredentialInputFragment dialog = IndividualCredentialInputFragment.newInstance(listItem);
                dialog.show(fmgr, listItem.name);
            });
        }
        else if(listItem.result == ProjectAttachWrapper.RESULT_CONFIG_DOWNLOAD_FAILED) {
            // download failed, can not continue from here.
            // if user wants to retry, need to go back to selection activity
            status.setVisibility(View.VISIBLE);
            status.setText(listItem.getResultDescription());
            resolveIv.setVisibility(View.GONE);
            statusPb.setVisibility(View.GONE);
            statusImage.setVisibility(View.VISIBLE);
            statusImage.setImageDrawable(getThemedDrawable(R.drawable.ic_baseline_clear,
                                                           R.drawable.ic_baseline_clear_white));
        }
        else {
            // failed
            status.setVisibility(View.VISIBLE);
            status.setText(listItem.getResultDescription());
            resolveIv.setVisibility(View.VISIBLE);
            itemWrapper.setOnClickListener(view -> {
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "BatchConflictListAdapter: start resolution dialog for: " + listItem.name);
                }
                IndividualCredentialInputFragment dialog = IndividualCredentialInputFragment.newInstance(listItem);
                dialog.show(fmgr, listItem.name);
            });
            statusPb.setVisibility(View.GONE);
            statusImage.setVisibility(View.VISIBLE);
            statusImage.setImageDrawable(getThemedDrawable(R.drawable.ic_baseline_clear,
                                                           R.drawable.ic_baseline_clear_white));
        }
        return v;
    }

    private Drawable getThemedDrawable(@DrawableRes final int lightDrawableRes,
                                       @DrawableRes final int darkDrawableRes) {
        final int defaultNightMode = AppCompatDelegate.getDefaultNightMode();
        final Drawable drawable;
        if (defaultNightMode == AppCompatDelegate.MODE_NIGHT_YES) {
            drawable = activity.getResources().getDrawable(darkDrawableRes);
        } else if (defaultNightMode == AppCompatDelegate.MODE_NIGHT_NO) {
            drawable = activity.getResources().getDrawable(lightDrawableRes);
        } else {
            // Assume either "Set by Battery Saver" or "System default" is selected
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
                drawable = activity.getResources().getDrawable(lightDrawableRes);
            } else {
                drawable = activity.getDrawable(darkDrawableRes);
            }
        }
        return drawable;
    }

    @Override
    public int getCount() {
        return entries.size();
    }
}
