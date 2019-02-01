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
import android.content.Context;
import android.text.format.Formatter;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import edu.berkeley.boinc.PrefsFragment;
import edu.berkeley.boinc.PrefsFragment.BoolOnClick;
import edu.berkeley.boinc.R;

import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.ArrayList;

public class PrefsListAdapter extends ArrayAdapter<PrefsListItemWrapper> {

    //private final String TAG = "PrefsListAdapter";
    private ArrayList<PrefsListItemWrapper> entries;
    private Activity activity;
    private PrefsFragment frag;

    public PrefsListAdapter(Activity a, PrefsFragment frag, int textViewResourceId, ArrayList<PrefsListItemWrapper> entries) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
        this.frag = frag;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {

        View v;
        LayoutInflater vi = (LayoutInflater) activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

        PrefsListItemWrapper listItem = entries.get(position);

        if(listItem.isCategory) { // item is category
            v = vi.inflate(R.layout.prefs_layout_listitem_category, null);
            TextView header = v.findViewById(R.id.category_header);
            header.setText(listItem.ID);
        }
        else { // item is element
            // CheckBoxes
            if(listItem instanceof PrefsListItemWrapperBool) {
                v = vi.inflate(R.layout.prefs_layout_listitem_bool, null);
                CheckBox cb = v.findViewById(R.id.checkbox);
                cb.setChecked(((PrefsListItemWrapperBool) listItem).getStatus());
                BoolOnClick listener = frag.new BoolOnClick(listItem.ID, cb);
                RelativeLayout wrapper = v.findViewById(R.id.checkbox_wrapper);
                wrapper.setClickable(true);
                wrapper.setOnClickListener(listener);
                TextView header = v.findViewById(R.id.checkbox_text);
                header.setText(((PrefsListItemWrapperBool) listItem).header);
            }
            // Number based items
            else if(listItem instanceof PrefsListItemWrapperNumber) {
                PrefsListItemWrapperNumber item = (PrefsListItemWrapperNumber) listItem;
                v = vi.inflate(R.layout.prefs_layout_listitem, null);
                RelativeLayout wrapper = v.findViewById(R.id.wrapper);
                wrapper.setOnClickListener(frag.new ValueOnClick(listItem));
                TextView header = v.findViewById(R.id.header);
                header.setText(item.header);
                TextView description = v.findViewById(R.id.description);
                description.setText(item.description);

                // set status value or hide if 0
                LinearLayout statusWrapper = v.findViewById(R.id.status_wrapper);
                if(item.status > 0) {
                    statusWrapper.setVisibility(View.VISIBLE);
                    final String value;
                    switch(item.unit) {
                        case NONE:
                            value = NumberFormat.getIntegerInstance().format(item.status);
                            break;
                        case PERCENT:
                            value = NumberFormat.getPercentInstance().format(item.status / 100.0);
                            break;
                        case SECONDS:
                            value = NumberFormat.getIntegerInstance().format(item.status) +
                                    this.activity.getString(R.string.prefs_unit_seconds);
                            break;
                        case CELSIUS:
                            value = NumberFormat.getInstance().format(item.status) +
                                    this.activity.getString(R.string.prefs_unit_celsius);
                            break;
                        case MEGABYTES:
                            value = Formatter.formatShortFileSize(this.activity, (long) (item.status * 0x100000));
                            break;
                        case GIGABYTES:
                            value = Formatter.formatShortFileSize(this.activity, (long) (item.status * 0x40000000));
                            break;
                        case DECIMAL:
                            value = DecimalFormat.getNumberInstance().format(item.status);
                            break;
                        default:
                            value = NumberFormat.getInstance().format(item.status);
                    }
                    ((TextView) v.findViewById(R.id.status)).setText(value);
                }
                else {
                    statusWrapper.setVisibility(View.GONE);
                }
            }
            // Text based items
            else if(listItem instanceof PrefsListItemWrapperText) {
                PrefsListItemWrapperText item = (PrefsListItemWrapperText) listItem;
                v = vi.inflate(R.layout.prefs_layout_listitem, null);
                RelativeLayout wrapper = v.findViewById(R.id.wrapper);
                wrapper.setOnClickListener(frag.new ValueOnClick(listItem));
                TextView header = v.findViewById(R.id.header);
                header.setText(item.header);
                TextView description = v.findViewById(R.id.description);
                description.setText(item.description);

                LinearLayout statusWrapper = v.findViewById(R.id.status_wrapper);
                statusWrapper.setVisibility(View.VISIBLE);
                final String value = item.status;
                ((TextView) v.findViewById(R.id.status)).setText(value);
            }
            // Lists
            else {
                v = vi.inflate(R.layout.prefs_layout_listitem, null);
                RelativeLayout wrapper = v.findViewById(R.id.wrapper);
                wrapper.setOnClickListener(frag.new ValueOnClick(listItem));
                TextView header = v.findViewById(R.id.header);
                header.setText(listItem.header);
                if(listItem.ID == R.string.prefs_client_log_flags_header) {
                    TextView description = v.findViewById(R.id.description);
                    description.setVisibility(View.GONE);
                    LinearLayout statusWrapper = v.findViewById(R.id.status_wrapper);
                    statusWrapper.setVisibility(View.GONE);
                }
                else if(listItem.ID == R.string.prefs_power_source_header) {
                    TextView description = v.findViewById(R.id.description);
                    description.setText(listItem.description);
                    LinearLayout statusWrapper = v.findViewById(R.id.status_wrapper);
                    statusWrapper.setVisibility(View.GONE);
                }
            }
        }

        return v;
    }
}
