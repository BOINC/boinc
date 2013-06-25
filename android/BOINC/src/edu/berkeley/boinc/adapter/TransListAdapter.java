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
import android.graphics.Bitmap;
import android.text.format.DateUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.TransActivity.TransferData;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.rpc.Transfer;
import edu.berkeley.boinc.rpc.CcStatus;
import edu.berkeley.boinc.utils.BOINCErrors;
import edu.berkeley.boinc.utils.BOINCUtils;
import edu.berkeley.boinc.utils.Logging;

public class TransListAdapter extends ArrayAdapter<TransferData> implements OnItemClickListener {
	
	private ArrayList<TransferData> entries;
	private CcStatus status;
    private Activity activity;
    
    public TransListAdapter(Activity activity, ListView listView, int textViewResourceId, ArrayList<TransferData> entries, CcStatus status) {
        super(activity, textViewResourceId, entries);
        this.entries = entries;
        this.status = status;
        this.activity = activity;
        
        listView.setAdapter(this);
        listView.setOnItemClickListener(this);
        listView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
    }
 
	@Override
	public int getCount() {
		return entries.size();
	}

	@Override
	public TransferData getItem(int position) {
		return entries.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}
    
	public String getName(int position) {
		return entries.get(position).transfer.name;
	}

	public String getProjectURL(int position) {
		return entries.get(position).transfer.project_url;
	}

	public int getProgress(int position) {
		Transfer transfer = getItem(position).transfer;
		float fBytesSent = transfer.bytes_xferred;
	    float fFileSize = transfer.nbytes;
	    double dBuffer = 0.0;
	    
	    // Curl apparently counts the HTTP header in byte count.
	    // Prevent this from causing > 100% display
	    
	    if (fBytesSent > fFileSize) {
	    	fBytesSent = fFileSize;
	    }
	    
	    if (fFileSize > 0.0) {
	    	dBuffer = Math.floor((fBytesSent / fFileSize) * 10000) / 100;
	    }
	    
	    if ( 0 == fFileSize ) return 0;
	    int progress = (int)Math.round(dBuffer);
		return progress;
	}

	public String getStatus(int position) {
		Transfer transfer = getItem(position).transfer;
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

		TransferData listItem = entries.get(position);

		View v = convertView;
		// setup new view, if:
		// - view is null, has not been here before
		// - view has different id
		Boolean setup = false;
		if(v == null) setup = true;
		else {
			String viewId = (String)v.getTag();
			if(!listItem.id.equals(viewId)) setup = true;
		}
		
		if(setup){
		    LayoutInflater li = (LayoutInflater)activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		    v = li.inflate(R.layout.trans_layout_listitem, null);
		    v.setOnClickListener(entries.get(position).transClickListener);
		    v.setTag(listItem.id);
		}
	    
	    ImageView ivIcon = (ImageView)v.findViewById(R.id.projectIcon);
	    Bitmap icon = getIcon(position);
	    // if available set icon, if not boinc logo
	    if (icon == null) { 
	    	ivIcon.setImageDrawable(getContext().getResources().getDrawable(R.drawable.boinc));
	    } else { 
	    	ivIcon.setImageBitmap(icon);
	    }
	    
	    TextView transferName = (TextView)v.findViewById(R.id.transName);
	    ProgressBar progressBar = (ProgressBar)v.findViewById(R.id.progressBar);
	    TextView statusText = (TextView)v.findViewById(R.id.transStatus);
	    
	    progressBar.setIndeterminate(false);
	    progressBar.setProgressDrawable(this.activity.getResources()
	    		.getDrawable((determineProgressBarLayout(listItem))));
	    progressBar.setProgress(getProgress(position));
	    
	    TextView header = (TextView) v.findViewById(R.id.transHeader);
	    String headerT = listItem.transfer.project_url;
	    
	    // try to get readable project name from ClientStatus
		try{
			ClientStatus status  = Monitor.getClientStatus();
		    ArrayList<Project> projects = status.getProjects();

		    //Does a search for the real project name 
		    int i = 0;
			for(i = 0; i < projects.size(); i++) {
				if(projects.get(i).master_url.equalsIgnoreCase(listItem.transfer.project_url)) {
					headerT = projects.get(i).getName();
				}
			}
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"TransListAdapter: Could not load data, clientStatus not initialized.");
		}
		
	    header.setText(headerT);
	    
	    // set project name
	    String tempProjectName = listItem.transfer.project_url;
	    ((TextView) v.findViewById(R.id.projectName)).setText(tempProjectName);
	    
	    String statusT = determineStatusText(listItem);
	    statusText.setText(statusT);
	    
	    //TODO: should there be elapsed time text?
	    
	    LinearLayout ll = (LinearLayout) v.findViewById(R.id.expansion);
	    if (listItem.expanded) {
	    	((ImageView)v.findViewById(R.id.expandCollapse)).setImageResource(R.drawable.collapse);
	    	ll.setVisibility(View.VISIBLE);
	    	
	    	transferName.setText(getName(position));
	    	
	    	if(listItem.determineState() == TransferData.TRANSFER_ABORTED) { //don't show buttons for aborted transfer
	    		// TODO: this state will never be reached, since nothing returns TRANSFER_ABORTED for now
	    		((LinearLayout)v.findViewById(R.id.requestPendingWrapper)).setVisibility(View.GONE);
	    		((LinearLayout)v.findViewById(R.id.transButtons)).setVisibility(View.INVISIBLE);
	    	} else {
	    		
	    		ImageView abortButton = (ImageView) v.findViewById(R.id.abortTrans);
	    		abortButton.setOnClickListener(listItem.iconClickListener);
	    		abortButton.setTag(RpcClient.TRANSFER_ABORT); // tag on button specified operation triggered in iconClickListener
	    		
	    		if (listItem.expectedState == -1) { // not waiting for new state
	    			((LinearLayout)v.findViewById(R.id.requestPendingWrapper)).setVisibility(View.GONE);
	    			((LinearLayout)v.findViewById(R.id.transButtons)).setVisibility(View.VISIBLE);
	    		} else {
	    			((LinearLayout)v.findViewById(R.id.transButtons)).setVisibility(View.INVISIBLE);
	    			((LinearLayout)v.findViewById(R.id.requestPendingWrapper)).setVisibility(View.VISIBLE);
	    		}
	    	}
	    } else {
	    	((ImageView)v.findViewById(R.id.expandCollapse)).setImageResource(R.drawable.expand);
	    	ll.setVisibility(View.GONE);
	    }
	    
	    return v;
	}
    
    public Bitmap getIcon(int position) {
		// try to get current client status from monitor
		ClientStatus status;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"TasksListAdapter: Could not load data, clientStatus not initialized.");
			return null;
		}
		return status.getProjectIcon(entries.get(position).transfer.project_url);
    }
    
    public void onItemClick(AdapterView<?> adapter, View view, int position, long id ) {
    }
    
    private String determineStatusText(TransferData tmp) {
    	Transfer transfer = tmp.transfer;
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
    			buf += BOINCUtils.translateNetworkSuspendReason(getContext(), status.network_suspend_reason);
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
    
    private Integer determineProgressBarLayout(TransferData tmp) {
    	return R.drawable.progressbar;
    }
}
