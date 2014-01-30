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
import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.ListView;
import edu.berkeley.boinc.PrefsFragment.SelectionDialogOption;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.Logging;

public class PrefsSelectionDialogListAdapter extends ArrayAdapter<SelectionDialogOption> implements OnClickListener {
	
	private ArrayList<SelectionDialogOption> entries;
    private Activity activity;
    
    public PrefsSelectionDialogListAdapter(Activity activity, ListView listView, int textViewResourceId, ArrayList<SelectionDialogOption> entries) {
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
    
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {

		View v = convertView;
		SelectionDialogOption listItem = entries.get(position);

		if(v == null) {
	        LayoutInflater li = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    		v = li.inflate(R.layout.dialog_list_cbitem, null);
    		CheckBox cb = (CheckBox) v.findViewById(R.id.checkbox);
    		cb.setChecked(listItem.selected);
    		cb.setOnClickListener(this);
    		cb.setTag(position);
    		cb.setText(listItem.name);
    		if(getItem(position).highlighted) {
    			v.setBackgroundDrawable(activity.getResources().getDrawable(R.drawable.shape_light_red_background));
    			cb.setBackgroundColor(activity.getResources().getColor(R.color.light_red));
    		}
		}
	    return v;
	}

	@Override
	public void onClick(View v) {
		if(Logging.DEBUG) Log.d(Logging.TAG,"PrefsSelectionDialogListAdapter onClick");
		CheckBox cb = (CheckBox) v;
		Integer position = (Integer) cb.getTag();
		Boolean checked = cb.isChecked();
		entries.get(position).selected = checked;
		
	}
}
