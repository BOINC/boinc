/*
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
 */
package edu.berkeley.boinc.attach;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.FragmentActivity;

import java.util.List;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.Logging;

public class SelectionListAdapter extends ArrayAdapter<ProjectListEntry> {
    private List<ProjectListEntry> entries;
    private FragmentActivity activity;

    public SelectionListAdapter(FragmentActivity a, int textViewResourceId, List<ProjectListEntry> entries) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
    }

    @NonNull
    @Override
    public View getView(int position, View convertView, @NonNull ViewGroup parent) {
        View v;
        LayoutInflater vi = ContextCompat.getSystemService(activity, LayoutInflater.class);
        final ProjectListEntry listItem = entries.get(position);

        assert vi != null;
        v = vi.inflate(R.layout.attach_project_list_layout_listitem, null);
        TextView name = v.findViewById(R.id.name);
        TextView description = v.findViewById(R.id.description);
        TextView summary = v.findViewById(R.id.summary);
        CheckBox cb = v.findViewById(R.id.cb);
        LinearLayout textWrapper = v.findViewById(R.id.text_wrapper);

        if(listItem.isAccountManager()) {
            // element is account manager
            name.setText(activity.getString(R.string.attachproject_acctmgr_header));
            description.setText(activity.getString(R.string.attachproject_acctmgr_list_desc));
            cb.setVisibility(View.GONE);
            summary.setVisibility(View.GONE);
            ImageView button = v.findViewById(R.id.am_button_image);
            button.setVisibility(View.VISIBLE);
            OnClickListener listener = view -> {
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "SelectionListAdapter: account manager clicked.");
                }
                AcctMgrFragment dialog = new AcctMgrFragment();
                dialog.setReturnToMainActivity(); // configure, so dialog returns to main activity when finished
                dialog.show(activity.getSupportFragmentManager(), activity.getString(R.string.attachproject_acctmgr_header));
            };
            v.setOnClickListener(listener);
            name.setOnClickListener(listener);
            description.setOnClickListener(listener);
            button.setOnClickListener(listener);
        }
        else {
            // element is project option
            name.setText(listItem.getInfo().getName());
            description.setText(listItem.getInfo().getGeneralArea());
            summary.setText(listItem.getInfo().getSummary());
            cb.setChecked(listItem.isChecked());
            cb.setOnClickListener(view -> listItem.setChecked(!listItem.isChecked()));
            textWrapper.setOnClickListener(view -> {
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "SelectionListAdapter: onProjectClick open info for: " +
                                       listItem.getInfo().getName());
                }

                ProjectInfoFragment dialog = ProjectInfoFragment.newInstance(listItem.getInfo());
                dialog.show(activity.getSupportFragmentManager(), "ProjectInfoFragment");
            });

        }
        return v;
    }

    @Override
    public int getCount() {
        return entries.size();
    }
}
