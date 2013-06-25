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

import java.util.ArrayList;

import edu.berkeley.boinc.adapter.GalleryAdapter;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.ClientStatus.ImageWrapper;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.DeviceStatus;
import edu.berkeley.boinc.utils.BOINCDefs;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Gallery;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class StatusActivity extends Activity implements OnClickListener{
	
	private Monitor monitor;
	private Boolean mIsBound = false;
	
	// keep computingStatus and suspend reason to only adapt layout when changes occur
	private Integer computingStatus = -1;
	private Integer suspendReason = -1;
	
	//slide show
    private RelativeLayout slideshowWrapper;
    private Integer screenHeight = 0;
    private Integer screenWidth = 0;
    private Integer minScreenHeightForSlideshow = 1000;
    private Integer minScreenHeightForImage = 1000;
    private ArrayList<ImageWrapper> slideshowImages = new ArrayList<ImageWrapper>();
    private GalleryAdapter galleryAdapter;
    private Activity activity = this;

	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context,Intent intent) {
			//if(Logging.DEBUG) Log.d(TAG+"-localClientStatusRecNoisy","received action " + intent.getAction());
			loadLayout(false); // load layout, function distincts whether there is something to do
		}
	};
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	
	// connection to Monitor Service.
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	    	if(Logging.VERBOSE) Log.v(Logging.TAG, "StatusActivity onServiceConnected");

	    	monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
		    loadLayout(true);
	    }

	    public void onServiceDisconnected(ComponentName className) {
	    	if(Logging.DEBUG) Log.d(Logging.TAG, "StatusActivity onServiceDisconnected");

	    	monitor = null;
	        mIsBound = false;
	    }
	};

	void doBindService() {
		if(!mIsBound) {
			getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, 0); //calling within Tab needs getApplicationContext() for bindService to work!
		}
	}

	void doUnbindService() {
	    if (mIsBound) {
	        getApplicationContext().unbindService(mConnection);
	        mIsBound = false;
	    }
	}
	
	public void onCreate(Bundle savedInstanceState) {
		//bind to monitor in order to call its functions and access ClientStatus singleton
		doBindService();
		super.onCreate(savedInstanceState);
	}
	
	public void onResume() {
		//register noisy clientStatusChangeReceiver here, so only active when Activity is visible
		if(Logging.VERBOSE) Log.v(Logging.TAG,"StatusActivity register receiver");
		registerReceiver(mClientStatusChangeRec,ifcsc);
		
		Display display = getWindowManager().getDefaultDisplay();
		screenWidth = display.getWidth();
		screenHeight = display.getHeight();
		if(Logging.DEBUG) Log.d(Logging.TAG,"screen dimensions: " + screenWidth + "*" + screenHeight);
		
		try{
			minScreenHeightForSlideshow = getResources().getInteger(R.integer.status_min_screen_height_for_slideshow_px);
			minScreenHeightForImage = getResources().getInteger(R.integer.status_min_screen_height_for_image_px);
		} catch(Exception e){}
		
		loadLayout(true);
		super.onResume();
	}
	
	public void onPause() {
		//unregister receiver, so there are not multiple intents flying in
		if(Logging.VERBOSE) Log.v(Logging.TAG,"StatusActivity remove receiver");
		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}

	@Override
	protected void onDestroy() {
	    doUnbindService();
	    super.onDestroy();
	}
	
	private void loadLayout(Boolean forceUpdate) {
		//load layout, if service is available and ClientStatus can be accessed.
		//if this is not the case, "onServiceConnected" will call "loadLayout" as soon as the service is bound
		if(mIsBound) {
			// try to get current client status from monitor
			ClientStatus status;
			try{
				status  = Monitor.getClientStatus();
			} catch (Exception e){
				if(Logging.WARNING) Log.w(Logging.TAG,"StatusActivity: Could not load data, clientStatus not initialized.");
				return;
			}
			
			// layout only if client RPC connection is established
			// otherwise BOINCActivity does not start Tabs
			if(status.setupStatus == ClientStatus.SETUP_STATUS_AVAILABLE) { 
				
				// return in cases nothing has changed
				if (!forceUpdate && computingStatus == status.computingStatus && computingStatus != ClientStatus.COMPUTING_STATUS_SUSPENDED) return; 
				if (!forceUpdate && computingStatus == status.computingStatus && computingStatus == ClientStatus.COMPUTING_STATUS_SUSPENDED && status.computingSuspendReason == suspendReason) return;
				
				// set layout and retrieve elements
				setContentView(R.layout.status_layout);
				LinearLayout centerWrapper = (LinearLayout) findViewById(R.id.center_wrapper);
				TextView statusHeader = (TextView) findViewById(R.id.status_header);
				ImageView statusImage = (ImageView) findViewById(R.id.status_image);
				TextView statusDescriptor = (TextView) findViewById(R.id.status_long);
				slideshowWrapper = (RelativeLayout) findViewById(R.id.slideshow_wrapper);
				
				// adapt to specific computing status
				switch(status.computingStatus) {
				case ClientStatus.COMPUTING_STATUS_NEVER:
					slideshowWrapper.setVisibility(View.GONE);
					statusHeader.setText(R.string.status_computing_disabled);
					statusImage.setImageResource(R.drawable.playb48);
					statusImage.setContentDescription(getString(R.string.status_computing_disabled));
					statusImage.setClickable(true);
					statusImage.setOnClickListener(this);
					statusDescriptor.setText(R.string.status_computing_disabled_long);
					centerWrapper.setVisibility(View.VISIBLE);
					break;
				case ClientStatus.COMPUTING_STATUS_SUSPENDED:
					slideshowWrapper.setVisibility(View.GONE);
					statusHeader.setText(R.string.status_paused);
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
						LinearLayout restartingWrapper = (LinearLayout) findViewById(R.id.restarting_wrapper);
						restartingWrapper.setVisibility(View.VISIBLE);
						statusDescriptor.setText(R.string.suspend_user_req);
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
							DeviceStatus deviceStatus = new DeviceStatus(getApplicationContext());
							deviceStatus.update();
							Integer currentCharge = deviceStatus.getBattery_charge_pct();
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
					slideshowWrapper.setVisibility(View.GONE);
					centerWrapper.setVisibility(View.VISIBLE);
					statusHeader.setText(R.string.status_idle);
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
					// check if screen is high enough for slideshow
					if(screenHeight < minScreenHeightForSlideshow) { 
						if(Logging.DEBUG) Log.d(Logging.TAG, "loadPlainOldComputingScreen, screen too small.");
						loadPlainOldComputingScreen();
					} else new UpdateSlideshowImagesAsync().execute();
					break;
				}
				computingStatus = status.computingStatus; //save new computing status
			} else { // BOINC client is not available
				//invalid computingStatus, forces layout on next event
				computingStatus = -1;
			}
		}
	}
	
	// load computing screen, if
	// no slideshow images available, or
	// screen too small for slideshow
	private void loadPlainOldComputingScreen() {
		LinearLayout centerWrapper = (LinearLayout) findViewById(R.id.center_wrapper);
		centerWrapper.setVisibility(View.VISIBLE);
		TextView statusHeader = (TextView) findViewById(R.id.status_header);
		ImageView statusImage = (ImageView) findViewById(R.id.status_image);
		TextView statusDescriptor = (TextView) findViewById(R.id.status_long);
		statusHeader.setText(R.string.status_running);
		statusImage.setImageResource(R.drawable.cogsb48);
		statusImage.setContentDescription(getString(R.string.status_running));
		statusDescriptor.setText(R.string.status_running_long);
	}

	@Override
	public void onClick(View v) {
		new WriteClientRunModeAsync().execute(BOINCDefs.RUN_MODE_AUTO);
	}
	
	private final class WriteClientRunModeAsync extends AsyncTask<Integer, Void, Boolean> {
		
		@Override
		protected Boolean doInBackground(Integer... params) {
			return monitor.setRunMode(params[0]);
		}
		
		@Override
		protected void onPostExecute(Boolean success) {
			if(success) monitor.forceRefresh();
			else if(Logging.WARNING) Log.w(Logging.TAG,"setting run mode failed");
		}
	}
	
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
	}
}
