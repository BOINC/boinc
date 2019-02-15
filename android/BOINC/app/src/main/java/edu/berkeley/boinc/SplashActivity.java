/*
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
 */
package edu.berkeley.boinc;

import edu.berkeley.boinc.attach.SelectionListActivity;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.IMonitor;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.utils.Logging;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.view.View.OnLongClickListener;
import android.widget.ImageView;

/**
 * Activity shown at start. Forwards to BOINCActivity automatically, once Monitor has connected to Client and received first data via RPCs.
 * This Activity can not be navigated to, it is also not part of the history stack.
 * Is also shown during shutdown.
 * Long click on the BOINC logo brings up the EventLog, in case their is a problem with the RPC connection that needs to be debugged.
 *
 * @author Joachim Fritzsch
 */
public class SplashActivity extends Activity {

    private Boolean mIsBound = false;
    private Activity activity = this;
    private static IMonitor monitor = null;

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been established
            mIsBound = true;
            monitor = IMonitor.Stub.asInterface(service);
            try {
                // check whether BOINC was able to acquire mutex
                if(!monitor.boincMutexAcquired()) {
                    showNotExclusiveDialog();
                }
                // read log level from monitor preferences and adjust accordingly
                Logging.setLogLevel(monitor.getLogLevel());
            }
            catch(RemoteException e) {
                Log.w(Logging.TAG, "initializing log level failed.");
            }
        }

        public void onServiceDisconnected(ComponentName className) {
            // This should not happen
            mIsBound = false;
            monitor = null;
        }
    };

    private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            //if(Logging.DEBUG) Log.d(Logging.TAG, "SplashActivity ClientStatusChange - onReceive()");

            if(mIsBound) {
                try {
                    int setupStatus = SplashActivity.monitor.getSetupStatus();
                    switch(setupStatus) {
                        case ClientStatus.SETUP_STATUS_AVAILABLE:
                            if(Logging.DEBUG) {
                                Log.d(Logging.TAG, "SplashActivity SETUP_STATUS_AVAILABLE");
                            }
                            // forward to BOINCActivity
                            Intent startMain = new Intent(activity, BOINCActivity.class);
                            startActivity(startMain);
                            break;
                        case ClientStatus.SETUP_STATUS_NOPROJECT:
                            if(Logging.DEBUG) {
                                Log.d(Logging.TAG, "SplashActivity SETUP_STATUS_NOPROJECT");
                            }
                            // run benchmarks to speed up project initialization
                            boolean benchmarks = monitor.runBenchmarks();
                            if(Logging.DEBUG) {
                                Log.d(Logging.TAG, "SplashActivity: runBenchmarks returned: " + benchmarks);
                            }
                            // forward to PROJECTATTACH
                            Intent startAttach = new Intent(activity, SelectionListActivity.class);
                            startActivity(startAttach);
                            break;
                        case ClientStatus.SETUP_STATUS_ERROR:
                            if(Logging.ERROR) {
                                Log.e(Logging.TAG, "SplashActivity SETUP_STATUS_ERROR");
                            }
                            // do not show log here. error is just a notification of timeout, which is followed by an intermediate (and indefinate) retry
                            break;
                    }
                }
                catch(Exception e) {
                    if(Logging.ERROR) {
                        Log.e(Logging.TAG, "SplashActivity.BroadcastReceiver.onReceive() error: ", e);
                    }
                }
            }
        }
    };
    private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash);

        // Use BOINC logo in Recent Apps Switcher
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) { // API 21
            String label = (String) activity.getTitle();
            Bitmap icon = BitmapFactory.decodeResource(activity.getResources(), R.drawable.boinc);

            activity.setTaskDescription(new ActivityManager.TaskDescription(label, icon));
        }

        //initialize logging with highest verbosity, read actual value when monitor connected.
        Logging.setLogLevel(5);

        //bind monitor service
        doBindService();

        // set long click listener to go to eventlog
        ImageView imageView = findViewById(R.id.logo);
        imageView.setOnLongClickListener(new OnLongClickListener() {

            @Override
            public boolean onLongClick(View v) {
                startActivity(new Intent(activity, EventLogActivity.class));
                return true;
            }
        });

    }

    @Override
    protected void onResume() { // gets called by system every time activity comes to front. after onCreate upon first creation
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "SplashActivity onResume()");
        }
        super.onResume();
        registerReceiver(mClientStatusChangeRec, ifcsc);
    }

    @Override
    protected void onPause() { // gets called by system every time activity loses focus.
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "SplashActivity onPause()");
        }
        super.onPause();
        unregisterReceiver(mClientStatusChangeRec);
    }

    @Override
    protected void onDestroy() {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "SplashActivity onDestroy()");
        }
        super.onDestroy();
        doUnbindService();
    }

    private void doBindService() {
        // start service to allow setForeground later on...
        startService(new Intent(this, Monitor.class));
        // Establish a connection with the service, onServiceConnected gets called when
        bindService(new Intent(this, Monitor.class), mConnection, Service.BIND_AUTO_CREATE);
    }

    private void doUnbindService() {
        if(mIsBound) {
            // Detach existing connection.
            unbindService(mConnection);
            mIsBound = false;
        }
    }

    private void showNotExclusiveDialog() {
        if(Logging.ERROR) {
            Log.e(Logging.TAG, "SplashActivity: another BOINC app found, show dialog.");
        }
        Intent notExclusiveDialogIntent = new Intent();
        notExclusiveDialogIntent.setClassName("edu.berkeley.boinc", "edu.berkeley.boinc.BoincNotExclusiveDialog");
        startActivity(notExclusiveDialogIntent);
        finish();
    }
}
