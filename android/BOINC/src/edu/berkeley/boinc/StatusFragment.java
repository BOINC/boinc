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

import edu.berkeley.boinc.utils.*;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.DeviceStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.utils.BOINCDefs;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

public class StatusFragment extends Fragment{
	
	// keep computingStatus and suspend reason to only adapt layout when changes occur
	private Integer computingStatus = -1;
	private Integer suspendReason = -1;
	private Integer setupStatus = -1;
	
	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context,Intent intent) {
			if(Logging.VERBOSE) Log.d(Logging.TAG, "StatusFragment ClientStatusChange - onReceive()"); 
			loadLayout(false);
		}
	};
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	
	public void onResume() {
		//register noisy clientStatusChangeReceiver here, so only active when Activity is visible
		if(Logging.VERBOSE) Log.v(Logging.TAG,"StatusFragment register receiver");
		getActivity().registerReceiver(mClientStatusChangeRec,ifcsc);
		
		//loadLayout(true);
		super.onResume();
	}
	
	public void onPause() {
		//unregister receiver, so there are not multiple intents flying in
		if(Logging.VERBOSE) Log.v(Logging.TAG,"StatusFragment remove receiver");
		getActivity().unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}
	
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    	if(Logging.VERBOSE) Log.v(Logging.TAG,"StatusFragment onCreateView");
        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.status_layout, container, false);
    }
	
	private void loadLayout(Boolean forceUpdate) {
		//load layout, if if ClientStatus can be accessed.
		//if this is not the case, the broadcast receiver will call "loadLayout" again
		ClientStatus status;
		DeviceStatus deviceStatus;
		try{
			status  = Monitor.getClientStatus();
			deviceStatus = Monitor.getDeviceStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"StatusFragment: Could not load data, clientStatus not initialized.");
			return;
		}
		
		// layout only if client RPC connection is established
		// otherwise BOINCActivity does not start Tabs
		if(status.setupStatus == ClientStatus.SETUP_STATUS_AVAILABLE) { 
			// return in cases nothing has changed
			if (!forceUpdate && computingStatus == status.computingStatus && computingStatus != ClientStatus.COMPUTING_STATUS_SUSPENDED) return; 
			if (!forceUpdate && computingStatus == status.computingStatus && computingStatus == ClientStatus.COMPUTING_STATUS_SUSPENDED && status.computingSuspendReason == suspendReason) return;
			
			// set layout and retrieve elements
			LinearLayout statusWrapper = (LinearLayout) getView().findViewById(R.id.status_wrapper);
			LinearLayout centerWrapper = (LinearLayout) getView().findViewById(R.id.center_wrapper);
			LinearLayout restartingWrapper = (LinearLayout) getView().findViewById(R.id.restarting_wrapper);
			TextView statusHeader = (TextView) getView().findViewById(R.id.status_header);
			ImageView statusImage = (ImageView) getView().findViewById(R.id.status_image);
			TextView statusDescriptor = (TextView) getView().findViewById(R.id.status_long);
			
			restartingWrapper.setVisibility(View.GONE);
			
			// adapt to specific computing status
			switch(status.computingStatus) {
			case ClientStatus.COMPUTING_STATUS_NEVER:
				statusWrapper.setVisibility(View.VISIBLE);
				statusHeader.setText(R.string.status_computing_disabled);
				statusHeader.setVisibility(View.VISIBLE);
				statusImage.setImageResource(R.drawable.playb48);
				statusImage.setContentDescription(getString(R.string.status_computing_disabled));
				statusDescriptor.setText(R.string.status_computing_disabled_long);
				centerWrapper.setVisibility(View.VISIBLE);
				centerWrapper.setOnClickListener(runModeOnClickListener);
				break;
			case ClientStatus.COMPUTING_STATUS_SUSPENDED:
				statusWrapper.setVisibility(View.VISIBLE);
				statusHeader.setText(R.string.status_paused);
				statusHeader.setVisibility(View.VISIBLE);
				statusImage.setImageResource(R.drawable.pauseb48);
				statusImage.setContentDescription(getString(R.string.status_paused));
				statusImage.setClickable(false);
				centerWrapper.setVisibility(View.VISIBLE);
				switch(status.computingSuspendReason) {
				case BOINCDefs.SUSPEND_REASON_BATTERIES:
					statusDescriptor.setText(R.string.suspend_batteries);
					statusImage.setImageResource(R.drawable.notconnectedb48);
					statusHeader.setVisibility(View.GONE);
					break;
				case BOINCDefs.SUSPEND_REASON_USER_ACTIVE:
					statusDescriptor.setText(R.string.suspend_useractive);
					break;
				case BOINCDefs.SUSPEND_REASON_USER_REQ:
					// state after user stops and restarts computation
					centerWrapper.setVisibility(View.GONE);
					restartingWrapper.setVisibility(View.VISIBLE);
					break;
				case BOINCDefs.SUSPEND_REASON_TIME_OF_DAY:
					statusDescriptor.setText(R.string.suspend_tod);
					break;
				case BOINCDefs.SUSPEND_REASON_BENCHMARKS:
					statusDescriptor.setText(R.string.suspend_bm);
					statusImage.setImageResource(R.drawable.watchb48);
					statusHeader.setVisibility(View.GONE);
					break;
				case BOINCDefs.SUSPEND_REASON_DISK_SIZE:
					statusDescriptor.setText(R.string.suspend_disksize);
					break;
				case BOINCDefs.SUSPEND_REASON_CPU_THROTTLE:
					statusDescriptor.setText(R.string.suspend_cputhrottle);
					break;
				case BOINCDefs.SUSPEND_REASON_NO_RECENT_INPUT:
					statusDescriptor.setText(R.string.suspend_noinput);
					break;
				case BOINCDefs.SUSPEND_REASON_INITIAL_DELAY:
					statusDescriptor.setText(R.string.suspend_delay);
					break;
				case BOINCDefs.SUSPEND_REASON_EXCLUSIVE_APP_RUNNING:
					statusDescriptor.setText(R.string.suspend_exclusiveapp);
					break;
				case BOINCDefs.SUSPEND_REASON_CPU_USAGE:
					statusDescriptor.setText(R.string.suspend_cpu);
					break;
				case BOINCDefs.SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED:
					statusDescriptor.setText(R.string.suspend_network_quota);
					break;
				case BOINCDefs.SUSPEND_REASON_OS:
					statusDescriptor.setText(R.string.suspend_os);
					break;
				case BOINCDefs.SUSPEND_REASON_WIFI_STATE:
					statusDescriptor.setText(R.string.suspend_wifi);
					break;
				case BOINCDefs.SUSPEND_REASON_BATTERY_CHARGING:
					String text = getString(R.string.suspend_battery_charging);
					try{
						Double minCharge = Monitor.getClientStatus().getPrefs().battery_charge_min_pct;
						Integer currentCharge = deviceStatus.getStatus().battery_charge_pct;
						text = getString(R.string.suspend_battery_charging_long) + " " + minCharge.intValue()
						+ "% (" + getString(R.string.suspend_battery_charging_current) + " " + currentCharge  + "%) "
						+ getString(R.string.suspend_battery_charging_long2);
					} catch (Exception e) {}
					statusDescriptor.setText(text);
					statusImage.setImageResource(R.drawable.batteryb48);
					statusHeader.setVisibility(View.GONE);
					break;
				case BOINCDefs.SUSPEND_REASON_BATTERY_OVERHEATED:
					statusDescriptor.setText(R.string.suspend_battery_overheating);
					statusImage.setImageResource(R.drawable.batteryb48);
					statusHeader.setVisibility(View.GONE);
					break;
				default:
					statusDescriptor.setText(R.string.suspend_unknown);
					break;
				}
				suspendReason = status.computingSuspendReason;
				break;
			case ClientStatus.COMPUTING_STATUS_IDLE: 
				statusWrapper.setVisibility(View.VISIBLE);
				centerWrapper.setVisibility(View.VISIBLE);
				statusHeader.setText(R.string.status_idle);
				statusHeader.setVisibility(View.VISIBLE);
				statusImage.setImageResource(R.drawable.pauseb48);
				statusImage.setContentDescription(getString(R.string.status_idle));
				statusImage.setClickable(false);
				Integer networkState = 0;
				try{
					networkState = status.networkSuspendReason;
				} catch (Exception e) {}
				if(networkState == BOINCDefs.SUSPEND_REASON_WIFI_STATE){
					// Network suspended due to wifi state
					statusDescriptor.setText(R.string.suspend_wifi);
				}else {
					statusDescriptor.setText(R.string.status_idle_long);
				}
				break;
			case ClientStatus.COMPUTING_STATUS_COMPUTING:
				statusWrapper.setVisibility(View.GONE);
				break;
			}
			computingStatus = status.computingStatus; //save new computing status
			setupStatus = -1; // invalidate to force update next time no project
		} else if (status.setupStatus == ClientStatus.SETUP_STATUS_NOPROJECT) {
			
			if(setupStatus != ClientStatus.SETUP_STATUS_NOPROJECT) {
				// set layout and retrieve elements
				LinearLayout statusWrapper = (LinearLayout) getView().findViewById(R.id.status_wrapper);
				LinearLayout centerWrapper = (LinearLayout) getView().findViewById(R.id.center_wrapper);
				LinearLayout restartingWrapper = (LinearLayout) getView().findViewById(R.id.restarting_wrapper);
				TextView statusHeader = (TextView) getView().findViewById(R.id.status_header);
				ImageView statusImage = (ImageView) getView().findViewById(R.id.status_image);
				TextView statusDescriptor = (TextView) getView().findViewById(R.id.status_long);

				statusWrapper.setVisibility(View.VISIBLE);
				restartingWrapper.setVisibility(View.GONE);
				centerWrapper.setVisibility(View.VISIBLE);
				centerWrapper.setOnClickListener(addProjectOnClickListener);
				statusImage.setImageResource(R.drawable.projectsb48);
				statusHeader.setVisibility(View.GONE);
				statusDescriptor.setText(R.string.status_noproject);
				setupStatus = ClientStatus.SETUP_STATUS_NOPROJECT;
				computingStatus = -1;
			}
			
		}
		else { // BOINC client is not available
			//invalid computingStatus, forces layout on next event
			setupStatus = -1;
			computingStatus = -1;
		}
	}
	
	private OnClickListener runModeOnClickListener = new OnClickListener(){
		@Override
		public void onClick(View v) {
			new WriteClientRunModeAsync().execute(BOINCDefs.RUN_MODE_AUTO);
		}
	};
	
	private OnClickListener addProjectOnClickListener = new OnClickListener(){
		@Override
		public void onClick(View v) {
			startActivity(new Intent(getActivity(), AttachProjectListActivity.class));
		}
	};
	
	private final class WriteClientRunModeAsync extends AsyncTask<Integer, Void, Boolean> {
		
		@Override
		protected Boolean doInBackground(Integer... params) {
			// setting provided mode for both, CPU computation and network.
			Boolean runMode = ((BOINCActivity)getActivity()).getMonitorService().clientInterface.setRunMode(params[0]);
			Boolean networkMode = ((BOINCActivity)getActivity()).getMonitorService().clientInterface.setNetworkMode(params[0]);
			return runMode && networkMode;
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			if(success) ((BOINCActivity)getActivity()).getMonitorService().forceRefresh();
			else if(Logging.WARNING) Log.w(Logging.TAG,"StatusFragment: setting run mode failed");
		}
	}
	/*
	private final class UpdateSlideshowImagesAsync extends AsyncTask<Void, Void, Boolean> {
		
		@Override
		protected Boolean doInBackground(Void... params) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "UpdateSlideshowImagesAsync updating images in new thread.");
			// try to get current client status from monitor
			ClientStatus status;
			try{
				status  = Monitor.getClientStatus();
			} catch (Exception e){
				if(Logging.WARNING) Log.w(Logging.TAG,"UpdateSlideshowImagesAsync: Could not load data, clientStatus not initialized.");
				return false;
			}
			// load slideshow images
			status.updateSlideshowImages(slideshowImages);
			if(slideshowImages == null || slideshowImages.size() == 0) return false;
			return true;
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "UpdateSlideshowImagesAsync success: " + success);
			
			// check whether computingStatus has changed in the meantime.
			if(computingStatus != ClientStatus.COMPUTING_STATUS_COMPUTING) return;
			
			if(success) {
				// images available, adapt layout
			    Gallery gallery = (Gallery) findViewById(R.id.gallery);
			    final ImageView imageView = (ImageView) findViewById(R.id.image_view);
			    final TextView imageDesc = (TextView)findViewById(R.id.image_description);
		        imageDesc.setText(slideshowImages.get(0).projectName);
				LinearLayout centerWrapper = (LinearLayout) findViewById(R.id.center_wrapper);
				centerWrapper.setVisibility(View.GONE);
		        slideshowWrapper.setVisibility(View.VISIBLE);
		        
		        //setup adapter
		        galleryAdapter = new GalleryAdapter(activity, slideshowImages);
		        gallery.setAdapter(galleryAdapter);
		        
		        // adapt layout according to screen size
				if(screenHeight < minScreenHeightForImage) {
					// screen is not high enough for large image
					imageView.setVisibility(View.GONE);
					RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.WRAP_CONTENT);
					params.addRule(RelativeLayout.CENTER_IN_PARENT, RelativeLayout.TRUE);
					LinearLayout galleryWrapper = (LinearLayout) findViewById(R.id.gallery_wrapper);
					galleryWrapper.setLayoutParams(params);
					galleryWrapper.setPadding(0, 0, 0, 5);
			        gallery.setOnItemClickListener(new OnItemClickListener() {
			            public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
			                imageDesc.setText(slideshowImages.get(position).projectName);
			            }
			        });
				} else {
					// screen is high enough, fully blown layout
			        imageView.setImageBitmap(slideshowImages.get(0).image);
			        gallery.setOnItemClickListener(new OnItemClickListener() {
			            public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
			                imageView.setImageBitmap(slideshowImages.get(position).image);
			                imageDesc.setText(slideshowImages.get(position).projectName);
			            }
			        });
				}
			} else loadPlainOldComputingScreen();
		}
	}*/
}
