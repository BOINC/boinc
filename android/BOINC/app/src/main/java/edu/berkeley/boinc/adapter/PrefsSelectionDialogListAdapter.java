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
package edu.berkeley.boinc.adapter;

import android.app.Activity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;

import java.util.List;
import java.util.Objects;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.Logging;

public class PrefsSelectionDialogListAdapter extends ArrayAdapter<SelectionDialogOption> implements OnClickListener {
    private List<SelectionDialogOption> entries;
    private Activity activity;

    public PrefsSelectionDialogListAdapter(Activity activity, ListView listView, int textViewResourceId,
                                           List<SelectionDialogOption> entries) {
        super(activity, textViewResourceId, entries);
        this.entries = entries;
        this.activity = activity;

        listView.setAdapter(this);
        listView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
    }

    @Override
    public int getCount() {
        return entries.size();
    }

    @Override
    public SelectionDialogOption getItem(int position) {
        return entries.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @NonNull
    @Override
    public View getView(int position, View convertView, @NonNull ViewGroup parent) {
        View v = convertView;
        SelectionDialogOption listItem = entries.get(position);

        if(v == null) {
            LayoutInflater li = ContextCompat.getSystemService(activity, LayoutInflater.class);
            assert li != null;
            v = li.inflate(R.layout.prefs_layout_listitem_bool, null);
            CheckBox cb = v.findViewById(R.id.checkbox);
            cb.setChecked(listItem.isSelected());
            cb.setClickable(false);
            TextView text = v.findViewById(R.id.checkbox_text);
            text.setText(listItem.getName());
            RelativeLayout wrapper = v.findViewById(R.id.checkbox_wrapper);
            wrapper.setClickable(true);
            wrapper.setOnClickListener(this);
            wrapper.setTag(position);
            if(Objects.requireNonNull(getItem(position)).isHighlighted()) {
                v.setBackgroundResource(R.drawable.shape_light_red_background_wo_stroke);
            }
        }
        return v;
    }

    @Override
    public void onClick(View v) {
        Log.d(Logging.TAG, "PrefsSelectionDialogListAdapter onClick");
        RelativeLayout wrapper = (RelativeLayout) v;
        Integer position = (Integer) v.getTag();
        CheckBox cb = wrapper.findViewById(R.id.checkbox);
        boolean previousState = cb.isChecked();
        cb.setChecked(!previousState);
        entries.get(position).setSelected(cb.isChecked());
    }
}
