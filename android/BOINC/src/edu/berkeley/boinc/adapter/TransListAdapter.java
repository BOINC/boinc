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
import edu.berkeley.boinc.rpc.Transfer;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ProgressBar;
import android.widget.TextView;

public class TransListAdapter extends ArrayAdapter<Transfer>{
	
	private final String TAG = "TransListAdapter";
	private ArrayList<Transfer> entries;
    private Activity activity;
 
    public TransListAdapter(Activity a, int textViewResourceId, ArrayList<Transfer> entries) {
        super(a, textViewResourceId, entries);
        this.entries = entries;
        this.activity = a;
    }
 
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
    	
    	//get file that which shall be presented
    	Transfer listItem = entries.get(position);
    	
    	//setup view
        View v = convertView;
        LayoutInflater vi = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        v = vi.inflate(R.layout.trans_layout_listitem, null);
        //instanciate layout elements
		ProgressBar pb = (ProgressBar) v.findViewById(R.id.transProgressBar);
		TextView name = (TextView) v.findViewById(R.id.transName);
		TextView progress = (TextView) v.findViewById(R.id.transProgress);
		TextView status = (TextView) v.findViewById(R.id.transStatus);
		//customize layout elements
		String nameS = listItem.name;
		name.setText(nameS);
		String progressS = listItem.bytes_xferred + " / " + listItem.nbytes;
		progress.setText(progressS);
		pb.setIndeterminate(false);
		Float fraction = Float.valueOf("0.0");
		try {
			fraction = listItem.nbytes / (float) listItem.bytes_xferred;
		} catch (Exception e) { //e.g. 0 division
			Log.e(TAG,"0 division?",e);
		}
		Log.d(TAG, listItem.nbytes + "/" + listItem.bytes_xferred + " equals fraction " + fraction);
		pb.setProgress(Math.round(fraction * pb.getMax()));
		String statusS = "";
		if(listItem.xfer_active) {
			Log.d(TAG, "transfer is active");
			statusS = "Transferring...";
			pb.setProgressDrawable(this.activity.getResources().getDrawable((R.drawable.progressbar_active)));
		} else {
			statusS = "Waiting...";
			pb.setProgressDrawable(this.activity.getResources().getDrawable((R.drawable.progressbar_paused)));
		}
		status.setText(statusS);
        return v;
    }
}
