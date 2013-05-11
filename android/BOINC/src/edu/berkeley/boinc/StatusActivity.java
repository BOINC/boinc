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

import java.util.ArrayList;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.utils.BOINCDefs;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.GestureDetector;
import android.view.GestureDetector.SimpleOnGestureListener;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.ViewFlipper;

public class StatusActivity extends Activity {
	
	private final String TAG = "BOINC StatusActivity";
	
	private Monitor monitor;
	private Boolean mIsBound = false;
	
	// keep computingStatus and suspend reason to only adapt layout when changes occur
	private Integer computingStatus = -1;
	private Integer suspendReason = -1;
	
	//slide show
    private ViewFlipper imageFrame;
    
    // gesture detection
    private final GestureDetector gdt = new GestureDetector(new GestureListener());
	private class GestureListener extends SimpleOnGestureListener {
		// values taken from example on Stackoverflow. seems appropriate.
	    private final int SWIPE_MIN_DISTANCE = 120;
	    private final int SWIPE_THRESHOLD_VELOCITY = 200;
	    @Override
	    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
	       if(e1.getX() - e2.getX() > SWIPE_MIN_DISTANCE && Math.abs(velocityX) > SWIPE_THRESHOLD_VELOCITY) {
	          //Log.d(TAG, "right to left...");
	          imageFrame.setInAnimation(getApplicationContext(), R.anim.in_from_right);
	          imageFrame.setOutAnimation(getApplicationContext(), R.anim.out_to_left);
              imageFrame.showNext();
	          return false;
	       } else if (e2.getX() - e1.getX() > SWIPE_MIN_DISTANCE && Math.abs(velocityX) >  SWIPE_THRESHOLD_VELOCITY) {
	          //Log.d(TAG, "left to right...");
	          imageFrame.setInAnimation(getApplicationContext(), R.anim.in_from_left);
	          imageFrame.setOutAnimation(getApplicationContext(), R.anim.out_to_right);
              imageFrame.showPrevious();
	          return false;
	       }
	       return false;
	    }
	}

	private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context,Intent intent) {
			//Log.d(TAG+"-localClientStatusRecNoisy","received action " + intent.getAction());
			loadLayout(); // load layout, function distincts whether there is something to do
		}
	};
	private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
	
	// connection to Monitor Service.
	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	    	Log.d(TAG, "onServiceConnected");

	    	monitor = ((Monitor.LocalBinder)service).getService();
		    mIsBound = true;
		    loadLayout();
	    }

	    public void onServiceDisconnected(ComponentName className) {
	    	Log.d(TAG, "onServiceDisconnected");

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
		Log.d(TAG+"-onResume","register receiver");
		registerReceiver(mClientStatusChangeRec,ifcsc);
		loadLayout();
		super.onResume();
	}
	
	public void onPause() {
		//unregister receiver, so there are not multiple intents flying in
		Log.d(TAG+"-onPause","remove receiver");
		unregisterReceiver(mClientStatusChangeRec);
		super.onPause();
	}

	@Override
	protected void onDestroy() {
	    doUnbindService();
	    super.onDestroy();
	}
	
	private void loadLayout() {
		//load layout, if service is available and ClientStatus can be accessed.
		//if this is not the case, "onServiceConnected" will call "loadLayout" as soon as the service is bound
		if(mIsBound) {
			// get data
			ClientStatus status = Monitor.getClientStatus();
			
			// layout only if client RPC connection is established
			// otherwise BOINCActivity does not start Tabs
			if(status.setupStatus == ClientStatus.SETUP_STATUS_AVAILABLE) { 
				
				// return in cases nothing has changed
				if (computingStatus == status.computingStatus && computingStatus != ClientStatus.COMPUTING_STATUS_SUSPENDED) return; 
				if (computingStatus == status.computingStatus && computingStatus == ClientStatus.COMPUTING_STATUS_SUSPENDED && status.computingSuspendReason == suspendReason) return;
				
				// set layout and retrieve elements
				setContentView(R.layout.status_layout);
				LinearLayout centerWrapper = (LinearLayout) findViewById(R.id.center_wrapper);
				TextView statusHeader = (TextView) findViewById(R.id.status_header);
				ImageView statusImage = (ImageView) findViewById(R.id.status_image);
				TextView statusDescriptor = (TextView) findViewById(R.id.status_long);
				ImageView changeRunmodeImage = (ImageView) findViewById(R.id.status_change_runmode_image);
				TextView changeRunmodeDescriptor = (TextView) findViewById(R.id.status_change_runmode_long);
		        imageFrame = (ViewFlipper) findViewById(R.id.slideshowFrame);
				
				// adapt to specific computing status
				switch(status.computingStatus) {
				case ClientStatus.COMPUTING_STATUS_NEVER:
					imageFrame.setVisibility(View.GONE);
					statusHeader.setText(R.string.status_computing_disabled);
					statusImage.setImageResource(R.drawable.stopw48);
					statusImage.setContentDescription(getString(R.string.status_computing_disabled));
					statusDescriptor.setText(R.string.status_computing_disabled_long);
					changeRunmodeImage.setImageResource(R.drawable.playw24);
					changeRunmodeImage.setContentDescription(getString(R.string.enable_computation));
					changeRunmodeImage.setTag(true);
					changeRunmodeDescriptor.setText(R.string.enable_computation);
					changeRunmodeDescriptor.setTag(true);
					break;
				case ClientStatus.COMPUTING_STATUS_SUSPENDED:
					imageFrame.setVisibility(View.GONE);
					statusHeader.setText(R.string.status_paused);
					statusImage.setImageResource(R.drawable.pausew48);
					statusImage.setContentDescription(getString(R.string.status_paused));
					changeRunmodeImage.setImageResource(R.drawable.stopw24);
					changeRunmodeImage.setContentDescription(getString(R.string.disable_computation));
					changeRunmodeImage.setTag(false);
					changeRunmodeDescriptor.setText(R.string.disable_computation);
					changeRunmodeDescriptor.setTag(false);
					switch(status.computingSuspendReason) {
					case BOINCDefs.SUSPEND_REASON_BATTERIES:
						statusDescriptor.setText(R.string.suspend_batteries);
						statusImage.setImageResource(R.drawable.notconnectedw48);
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
						statusImage.setImageResource(R.drawable.watchw48);
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
						statusDescriptor.setText(R.string.suspend_battery_charging);
						statusImage.setImageResource(R.drawable.batteryw48);
						statusHeader.setVisibility(View.GONE);
						break;
					case BOINCDefs.SUSPEND_REASON_BATTERY_OVERHEATED:
						statusDescriptor.setText(R.string.suspend_battery_overheating);
						statusImage.setImageResource(R.drawable.batteryw48);
						statusHeader.setVisibility(View.GONE);
						break;
					default:
						statusDescriptor.setText(R.string.suspend_unknown);
						break;
					}
					suspendReason = status.computingSuspendReason;
					break;
				case ClientStatus.COMPUTING_STATUS_IDLE: 
					imageFrame.setVisibility(View.GONE);
					statusHeader.setText(R.string.status_idle);
					statusImage.setImageResource(R.drawable.pausew48);
					statusImage.setContentDescription(getString(R.string.status_idle));
					changeRunmodeImage.setImageResource(R.drawable.stopw24);
					changeRunmodeImage.setContentDescription(getString(R.string.disable_computation));
					changeRunmodeImage.setTag(false);
					changeRunmodeDescriptor.setText(R.string.disable_computation);
					changeRunmodeDescriptor.setTag(false);
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
					// load slideshow
					if(!loadSlideshow()) {
						Log.d(TAG, "slideshow not available, load plain old status instead...");
						statusHeader.setText(R.string.status_running);
						statusImage.setImageResource(R.drawable.playw48);
						statusImage.setContentDescription(getString(R.string.status_running));
						statusDescriptor.setText(R.string.status_running_long);
					}
					changeRunmodeImage.setImageResource(R.drawable.stopw24);
					changeRunmodeImage.setContentDescription(getString(R.string.disable_computation));
					changeRunmodeImage.setTag(false);
					changeRunmodeDescriptor.setText(R.string.disable_computation);
					changeRunmodeDescriptor.setTag(false);
					break;
				}
				computingStatus = status.computingStatus; //save new computing status
			} else { // BOINC client is not available
				//invalid computingStatus, forces layout on next event
				computingStatus = -1;
			}
		}
	}
	
	public void onClick (View v) {
		Log.d(TAG,"onClick");
		if(!mIsBound) {Log.w(TAG,"not bound");return;}
		try {
			Boolean enable = (Boolean) v.getTag();
			if(enable) {
				monitor.setRunMode(BOINCDefs.RUN_MODE_AUTO);
			} else {
				monitor.setRunMode(BOINCDefs.RUN_MODE_NEVER);
			}
		} catch (Exception e) {Log.e(TAG, "could not map status tag", e);}
	}
	
	private Boolean loadSlideshow() {
		// get slideshow images
		ArrayList<Bitmap> images = Monitor.getClientStatus().getSlideshowImages();
		if(images == null || images.size() == 0) return false;
		
		// images available, adapt layout
		LinearLayout centerWrapper = (LinearLayout) findViewById(R.id.center_wrapper);
		centerWrapper.setVisibility(View.GONE);
        imageFrame.setVisibility(View.VISIBLE);
        LayoutParams params = new LayoutParams(LayoutParams.FILL_PARENT,LayoutParams.FILL_PARENT);
        imageFrame.removeAllViews();
        
        // create views for all available bitmaps
    	for (Bitmap image: images) {
            ImageView imageView = new ImageView(this);
            imageView.setLayoutParams(params);
            imageView.setScaleType(ImageView.ScaleType.FIT_CENTER);
            imageView.setImageBitmap(image);
            imageFrame.addView(imageView);
    	}
        
        // capture click events and pass on to Gesture Detector
        imageFrame.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(final View view, final MotionEvent event) {
                gdt.onTouchEvent(event);
                return true;
            }
         });
        
        return true;
	}
}
