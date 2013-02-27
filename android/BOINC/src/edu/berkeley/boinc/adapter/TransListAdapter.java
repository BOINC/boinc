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
import java.util.Calendar;

import android.app.Activity;
import android.content.Context;
import android.text.format.DateUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import edu.berkeley.boinc.TransActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.Transfer;
import edu.berkeley.boinc.rpc.CcStatus;
import edu.berkeley.boinc.utils.BOINCErrors;
import edu.berkeley.boinc.utils.BOINCUtils;

public class TransListAdapter extends ArrayAdapter<Transfer> implements OnItemClickListener {
	
	private ArrayList<Transfer> entries;
	private CcStatus status;
    private Activity activity;
    private ListView listView;

    public static class ViewTransfer {
    	int entryIndex;
        TextView tvName;
        TextView tvProgress;
        TextView tvStatus;
        ProgressBar pbProgressBar;
        ImageButton ibRetry;
        ImageButton ibAbort;
    }
    
    public TransListAdapter(Activity activity, ListView listView, int textViewResourceId, ArrayList<Transfer> entries, CcStatus status) {
        super(activity, textViewResourceId, entries);
        this.entries = entries;
        this.status = status;
        this.activity = activity;
        this.listView = listView;
        
        listView.setAdapter(this);
        listView.setOnItemClickListener(this);
        listView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
    }
 
	@Override
	public int getCount() {
		return entries.size();
	}

	@Override
	public Transfer getItem(int position) {
		return entries.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}
    
	public String getName(int position) {
		return entries.get(position).name;
	}

	public String getProjectURL(int position) {
		return entries.get(position).project_url;
	}

	public int getProgress(int position) {
		Transfer transfer = getItem(position);
		float fBytesSent = transfer.bytes_xferred;
	    float fFileSize = transfer.nbytes;
	    double dBuffer = 0.0;
	    
	    // Curl apparently counts the HTTP header in byte count.
	    // Prevent this from causing > 100% display
	    //
	    if (fBytesSent > fFileSize) {
	        fBytesSent = fFileSize;
	    }

	    if (fFileSize > 0.0) {
	    	dBuffer = Math.floor((fBytesSent / fFileSize) * 10000)/100;
	    }
		
	    if (0 == fFileSize) return 0;
		return (int)Math.round(dBuffer);
	}

	public String getStatus(int position) {
		Transfer transfer = getItem(position);
		String buf = new String();

        Calendar nextRequest = Calendar.getInstance();
        Calendar now = Calendar.getInstance();
        nextRequest.setTimeInMillis((long)transfer.next_request_time*1000);

        if (transfer.is_upload) {
        	buf += activity.getResources().getString(R.string.trans_upload);
        } else {
        	buf += activity.getResources().getString(R.string.trans_download);
        }
        buf += ": ";
        if (nextRequest.compareTo(now) > 0) {
        	buf += activity.getResources().getString(R.string.trans_retryin);
        	buf += " ";
        	buf += DateUtils.formatElapsedTime((nextRequest.getTimeInMillis() - now.getTimeInMillis()) / 1000);
        } else if (transfer.status == BOINCErrors.ERR_GIVEUP_DOWNLOAD || transfer.status == BOINCErrors.ERR_GIVEUP_UPLOAD) {
        	buf = activity.getResources().getString(R.string.trans_failed);
        } else {
        	if (status.network_suspend_reason > 0) {
	            buf += activity.getResources().getString(R.string.trans_suspended);
	            buf += " - ";
	            buf += BOINCUtils.translateRPCReason(activity, status.network_suspend_reason);
        	} else {
	            if (transfer.xfer_active) {
	                buf += activity.getResources().getString(R.string.trans_active);
	            } else {
	                buf += activity.getResources().getString(R.string.trans_pending);
	            }
	        }
        }
	    if (transfer.project_backoff > 0) {
	        buf += " (";
	        buf += activity.getResources().getString(R.string.trans_projectbackoff);
	        buf += ": ";
	        buf += DateUtils.formatElapsedTime(transfer.project_backoff);
	        buf += ")";
	    }

		return buf;
	}    
    
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
	    View vi = convertView;
	    ViewTransfer viewTransfer;
    	
		// Only inflate a new view if the ListView does not already have a view assigned.
	    if (convertView == null) {
	    	
	    	vi = ((LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.trans_layout_listitem, null);

	    	viewTransfer = new ViewTransfer();
	    	viewTransfer.tvName = (TextView)vi.findViewById(R.id.trans_name);
	    	viewTransfer.tvProgress = (TextView)vi.findViewById(R.id.trans_progress);
	    	viewTransfer.pbProgressBar = (ProgressBar)vi.findViewById(R.id.trans_progressbar);
	    	viewTransfer.tvStatus = (TextView)vi.findViewById(R.id.trans_status);
	    	viewTransfer.ibRetry = (ImageButton)vi.findViewById(R.id.trans_retry);
	    	viewTransfer.ibAbort = (ImageButton)vi.findViewById(R.id.trans_abort);
	    
	        vi.setTag(viewTransfer);
	        
	    } else {
	    	
	    	viewTransfer = (ViewTransfer)vi.getTag();
	    	
	    }

		// Populate UI Elements
	    viewTransfer.entryIndex = position;
	    viewTransfer.tvName.setText(getName(position));
	    viewTransfer.tvProgress.setText(BOINCUtils.formatSize(getItem(position).bytes_xferred, getItem(position).nbytes));

	    viewTransfer.pbProgressBar.setIndeterminate(false);
	    viewTransfer.pbProgressBar.setProgress(getProgress(position));
		if(getItem(position).xfer_active) {
			viewTransfer.pbProgressBar.setProgressDrawable(this.activity.getResources().getDrawable((R.drawable.progressbar_active)));
		} else {
			viewTransfer.pbProgressBar.setProgressDrawable(this.activity.getResources().getDrawable((R.drawable.progressbar_paused)));
		}

	    viewTransfer.tvStatus.setText(getStatus(position));
	    if (listView.isItemChecked(position)) {
	    	viewTransfer.ibRetry.setVisibility(View.VISIBLE);
	    	viewTransfer.ibRetry.setTag(viewTransfer);
	    	viewTransfer.ibRetry.setClickable(true);
	    	viewTransfer.ibRetry.setOnClickListener(new OnClickListener() {
	            public void onClick(View v) {
	            	ViewTransfer viewTransfer = (ViewTransfer)v.getTag();
	            	TransActivity a = (TransActivity)activity;
	            	
	            	a.onTransferRetry(getProjectURL(viewTransfer.entryIndex), getName(viewTransfer.entryIndex));
	            }
	        });
	    		    		    	
	    	viewTransfer.ibAbort.setVisibility(View.VISIBLE);
	    	viewTransfer.ibAbort.setTag(viewTransfer);
	    	viewTransfer.ibAbort.setClickable(true);
	    	viewTransfer.ibAbort.setOnClickListener(new OnClickListener() {
	            public void onClick(View v) {
	            	ViewTransfer viewTransfer = (ViewTransfer)v.getTag();
	            	TransActivity a = (TransActivity)activity;
	            	
	            	a.onTransferAbort(getProjectURL(viewTransfer.entryIndex), getName(viewTransfer.entryIndex));
	            }
	        });
	    } else {
	    	viewTransfer.ibRetry.setVisibility(View.GONE);	    	
	    	viewTransfer.ibRetry.setClickable(false);
	    	viewTransfer.ibRetry.setOnClickListener(null);
	    	
	    	viewTransfer.ibAbort.setVisibility(View.GONE);
	    	viewTransfer.ibAbort.setClickable(false);
	    	viewTransfer.ibAbort.setOnClickListener(null);
	    }

        return vi;
    }

    
    public void onItemClick(AdapterView<?> adapter, View view, int position, long id ) {
	    ViewTransfer viewTransfer = (ViewTransfer)view.getTag();
    	((TransActivity)activity).onTransferClicked(getProjectURL(viewTransfer.entryIndex), getName(viewTransfer.entryIndex));
		notifyDataSetChanged();
    }
}
