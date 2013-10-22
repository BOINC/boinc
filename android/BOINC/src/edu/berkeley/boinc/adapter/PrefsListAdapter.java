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

import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.TextView;

public class PrefsListAdapter extends ArrayAdapter<PrefsListItemWrapper>{
	
	//private final String TAG = "PrefsListAdapter";
	private ArrayList<PrefsListItemWrapper> entries;
    private Activity activity;
 
    public PrefsListAdapter(Activity a, int textViewResourceId, ArrayList<PrefsListItemWrapper> entries) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
    }
 
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
    	
        View v = convertView;
        LayoutInflater vi = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        
    	PrefsListItemWrapper listItem = entries.get(position);
    	
    	if(listItem.isCategory) { // item is category
    		v = vi.inflate(R.layout.prefs_layout_listitem_category, null);
    		TextView header = (TextView) v.findViewById(R.id.category_header);
    		header.setText(listItem.ID);
    	} else { // item is element
	    	if(listItem instanceof PrefsListItemWrapperBool) {
	    		v = vi.inflate(R.layout.prefs_layout_listitem_bool, null);
	    		CheckBox header = (CheckBox) v.findViewById(R.id.checkbox);
	    		header.setText(((PrefsListItemWrapperBool) listItem).header);
	    		header.setTag(listItem.ID); //set ID as tag to checkbox, since checkbox is clicked
	        	header.setChecked(((PrefsListItemWrapperBool) listItem).getStatus());
	    	} else if(listItem instanceof PrefsListItemWrapperValue) {
	    		PrefsListItemWrapperValue item = (PrefsListItemWrapperValue) listItem;
	    		v = vi.inflate(R.layout.prefs_layout_listitem, null);
	    		v.setTag(listItem); //set listItem as tag to view, since root layout defines onClick method
	    		TextView header = (TextView) v.findViewById(R.id.header);
	    		header.setText(item.header);
	    		TextView description = (TextView) v.findViewById(R.id.description);
	    		description.setText(item.description.toString());
	    		
	    		String value = item.status.toString();
	    		if(item.isPct || item.isNumber) {
	    			value = "" + item.status.intValue();
	    		} 
	    		TextView status = (TextView) v.findViewById(R.id.status);
	    		status.setText(value + " " + item.unit);
	    	} else {
	    		// instance of PrefsListItemWrapper, e.g. client log flags
	    		PrefsListItemWrapper item = listItem;
	    		v = vi.inflate(R.layout.prefs_layout_listitem, null);
	    		v.setTag(listItem); //set listItem as tag to view, since root layout defines onClick method
	    		TextView header = (TextView) v.findViewById(R.id.header);
	    		header.setText(item.header);
	    		// hide unused views
	    		TextView description = (TextView) v.findViewById(R.id.description);
	    		description.setVisibility(View.GONE);
	    		TextView status = (TextView) v.findViewById(R.id.status);
	    		status.setVisibility(View.INVISIBLE);
	    	}
    	}
    	
        return v;
    }
}
