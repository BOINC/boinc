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

package edu.berkeley.boinc.attach;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.*;
import android.app.Dialog;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;

public class ManualUrlInputFragment extends DialogFragment{
	
	private EditText urlInputET;
    
    @Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.attach_project_manual_url_input_dialog, container, false);
        
        urlInputET = (EditText) v.findViewById(R.id.url_input);
        
        Button continueButton = (Button) v.findViewById(R.id.continue_button);
        continueButton.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				if(Logging.DEBUG) Log.d(Logging.TAG, "ManualUrlInputFragment: continue clicked");
				
				//TODO verify input
				//startActivity
	    		Intent intent = new Intent(getActivity(), BatchConflictListActivity.class);
	    		intent.putExtra("conflicts", false);
	    		intent.putExtra("manualUrl", urlInputET.getText().toString());
	    		startActivity(intent);
				dismiss();
			}
        });
        
        return v;
	}

	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState) {
		  Dialog dialog = super.onCreateDialog(savedInstanceState);

		  // request a window without the title
		  dialog.getWindow().requestFeature(Window.FEATURE_NO_TITLE);
		  return dialog;
	}
}
