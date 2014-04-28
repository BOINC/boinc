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
package edu.berkeley.boinc;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;

/**
 * Shows dialog to forward or exit if other BONIC based application detected on device.
 */
public class ForwardDialog extends Activity {
	 @Override
	    public void onCreate(Bundle savedInstanceState) {
		   super.onCreate(savedInstanceState);
			AlertDialog.Builder builder = new AlertDialog.Builder(this);
		    builder.setMessage(getString(R.string.ptg_dialog_text))
		    .setCancelable(false)
		    .setTitle(getString(R.string.ptg_dialog_header))
		    .setPositiveButton(getString(R.string.ptg_dialog_launch), new DialogInterface.OnClickListener() {
		        public void onClick(DialogInterface dialog, int id) {
		            Intent startPTGIntent = new Intent();
		            startPTGIntent.setClassName("com.htc.ptg", "com.htc.ptg.SplashActivity");
		            startActivity(startPTGIntent);
		            finish();
		           }
		       })
		    .setNegativeButton(getString(R.string.ptg_dialog_exit), new DialogInterface.OnClickListener() {
		        public void onClick(DialogInterface dialog, int id) {
		            //BOINCActivity.this.finish();
		      	  	finish();
		       }
		    }).show();
    
	 }
}
