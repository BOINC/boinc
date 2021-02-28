/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2021 University of California
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

import android.app.ActivityManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;

import java.util.concurrent.Callable;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import edu.berkeley.boinc.attach.SelectionListActivity;
import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.IMonitor;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.databinding.ActivitySplashBinding;
import edu.berkeley.boinc.ui.eventlog.EventLogActivity;
import edu.berkeley.boinc.utils.BOINCUtils;
import edu.berkeley.boinc.utils.Logging;

/**
 * Activity shown at start. Forwards to BOINCActivity automatically, once Monitor has connected to Client and received first data via RPCs.
 * This Activity can not be navigated to, it is also not part of the history stack.
 * Is also shown during shutdown.
 * Long click on the BOINC logo brings up the EventLog, in case their is a problem with the RPC connection that needs to be debugged.
 *
 * @author Joachim Fritzsch
 */
public class SplashActivity extends AppCompatActivity {
    private ActivitySplashBinding binding;

    private boolean mIsBound = false;
    private static IMonitor monitor = null;

    private boolean mIsWelcomeSpecificFirstRun = true;

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been established
            mIsBound = true;
            monitor = IMonitor.Stub.asInterface(service);
            try {
                // check whether BOINC was able to acquire mutex
                if(!monitor.boincMutexAcquired()) {
                    showNotExclusiveDialog();
                }
                mIsWelcomeSpecificFirstRun =
                        BuildConfig.BUILD_TYPE.contains("xiaomi") && !monitor.getWelcomeStateFile();
                // read log level from monitor preferences and adjust accordingly
                Logging.setLogLevel(monitor.getLogLevel());
            }
            catch(RemoteException e) {
                Log.w(Logging.TAG, "initializing log level failed.");
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            // This should not happen
            mIsBound = false;
            monitor = null;
        }
    };

    public static class TaskRunner {
        private final Executor executor = Executors.newSingleThreadExecutor(); // change according to your requirements
        private final Handler handler = new Handler(Looper.getMainLooper());

        public interface Callback<R> {
            void onComplete(R result);
        }

        public <R> void executeAsync(Callable<R> callable, Callback<R> callback) {
            executor.execute(() -> {
                try {
                    final R result = callable.call();
                    handler.post(() -> {
                        callback.onComplete(result);
                    });
                }catch(Exception e){
                    Log.d(Logging.TAG, e.getMessage());
                    e.printStackTrace();
                }
            });
        }
    }

    class BenchmarksTask implements Callable<Boolean> {
        @Override
        public Boolean call() {
            // Some long running task
            Boolean benchmarks = false;
            try {
                benchmarks = monitor.runBenchmarks();
            }
            catch(RemoteException e) {
                e.printStackTrace();
            }
            return benchmarks;
        }
    }

    private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if(mIsBound) {
                try {
                    if (mIsWelcomeSpecificFirstRun) {
                        startActivity(new Intent(SplashActivity.this, LicenseActivity.class));
                        return;
                    }
                    int setupStatus = SplashActivity.monitor.getSetupStatus();
                    switch(setupStatus) {
                        case ClientStatus.SETUP_STATUS_AVAILABLE:
                            if(Logging.DEBUG) {
                                Log.d(Logging.TAG, "SplashActivity SETUP_STATUS_AVAILABLE");
                            }
                            // forward to BOINCActivity
                            Intent startMain = new Intent(SplashActivity.this, BOINCActivity.class);
                            startActivity(startMain);
                            break;
                        case ClientStatus.SETUP_STATUS_NOPROJECT:
                            if(Logging.DEBUG) {
                                Log.d(Logging.TAG, "SplashActivity SETUP_STATUS_NOPROJECT");
                            }
                            // run benchmarks to speed up project initialization
                            TaskRunner taskRunner = new TaskRunner();
                            taskRunner.executeAsync(new BenchmarksTask(), (benchmarks) -> {
                                if(Logging.DEBUG) {
                                    Log.d(Logging.TAG, "SplashActivity: runBenchmarks returned: " + benchmarks);
                                }
                            });

                            // forward to PROJECTATTACH
                            Intent startAttach = new Intent(SplashActivity.this, SelectionListActivity.class);
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
        binding = ActivitySplashBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Use BOINC logo in Recent Apps Switcher
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) { // API 21
            final String label = getTitle().toString();
            final ActivityManager.TaskDescription taskDescription;

            if(Build.VERSION.SDK_INT < Build.VERSION_CODES.P) { // API 28
                Bitmap icon = BOINCUtils.getBitmapFromVectorDrawable(this, R.drawable.ic_boinc);
                taskDescription = new ActivityManager.TaskDescription(label, icon);
            } else {
                taskDescription = new ActivityManager.TaskDescription(label, R.drawable.ic_boinc);
            }

            setTaskDescription(taskDescription);
        }

        //initialize logging with highest verbosity, read actual value when monitor connected.
        Logging.setLogLevel(5);

        //bind monitor service
        doBindService();

        // set long click listener to go to eventlog
        binding.logo.setOnLongClickListener(view -> {
            startActivity(new Intent(SplashActivity.this, EventLogActivity.class));
            return true;
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
