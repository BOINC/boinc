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

import java.util.ArrayList;

import edu.berkeley.boinc.adapter.ClientLogListAdapter;
import edu.berkeley.boinc.client.IMonitor;
import edu.berkeley.boinc.client.Monitor;
import edu.berkeley.boinc.rpc.Message;
import edu.berkeley.boinc.utils.*;

import android.content.ClipData;
import android.content.ComponentName;
import android.content.ClipboardManager;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.app.ActionBar.Tab;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MenuInflater;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

public class EventLogActivity extends AppCompatActivity {

    private IMonitor monitor;
    private Boolean mIsBound = false;

    public EventLogClientFragment clientFrag;
    public ListView clientLogList;
    public ClientLogListAdapter clientLogListAdapter;
    public ArrayList<Message> clientLogData = new ArrayList<>();

    public EventLogGuiFragment guiFrag;
    public ListView guiLogList;
    public ArrayAdapter<String> guiLogListAdapter;
    public ArrayList<String> guiLogData = new ArrayList<>();

    private ArrayList<EventLogActivityTabListener<?>> listener = new ArrayList<>();

    final static int GUI_LOG_TAB_ACTIVE = 1;
    final static int CLIENT_LOG_TAB_ACTIVE = 2;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        // setup action bar
        ActionBar actionBar = getSupportActionBar();
        actionBar.setTitle(R.string.menu_eventlog);

        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);

        EventLogActivityTabListener<EventLogClientFragment> clientListener =
                new EventLogActivityTabListener<>(this, getString(R.string.eventlog_client_header), EventLogClientFragment.class);
        listener.add(clientListener);
        Tab tab = actionBar.newTab()
                           .setText(R.string.eventlog_client_header)
                           .setTabListener(clientListener);
        actionBar.addTab(tab);

        EventLogActivityTabListener<EventLogGuiFragment> guiListener =
                new EventLogActivityTabListener<>(this, getString(R.string.eventlog_gui_header), EventLogGuiFragment.class);
        listener.add(guiListener);
        tab = actionBar.newTab()
                       .setText(R.string.eventlog_gui_header)
                       .setTabListener(guiListener);
        actionBar.addTab(tab);

        actionBar.setDisplayHomeAsUpEnabled(true);

        doBindService();
    }

    @Override
    protected void onDestroy() {
        doUnbindService();
        super.onDestroy();
    }

    /*
     * Service binding part
     * only necessary, when function on monitor instance has to be called
     */
    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            if(Logging.VERBOSE) {
                Log.d(Logging.TAG, "EventLogActivity onServiceConnected");
            }
            monitor = IMonitor.Stub.asInterface(service);
            mIsBound = true;

            // initialize default fragment
            ((EventLogClientFragment) getSupportFragmentManager().findFragmentByTag(getString(R.string.eventlog_client_header))).init();
        }

        public void onServiceDisconnected(ComponentName className) {
            monitor = null;
            mIsBound = false;
        }
    };

    private void doBindService() {
        if(!mIsBound) {
            getApplicationContext().bindService(new Intent(this, Monitor.class), mConnection, 0); //calling within Tab needs getApplicationContext() for bindService to work!
        }
    }

    private void doUnbindService() {
        if(mIsBound) {
            getApplicationContext().unbindService(mConnection);
            mIsBound = false;
        }
    }

    public IMonitor getMonitorService() {
        if(!mIsBound) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG, "Fragment trying to obtain serive reference, but Monitor not bound in EventLogActivity");
            }
        }
        return monitor;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.eventlog_menu, menu);

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case R.id.refresh:
                updateCurrentFragment();
                return true;
            case R.id.email_to:
                onEmailTo();
                return true;
            case R.id.copy:
                onCopy();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private int getActiveLog() {
        for(EventLogActivityTabListener<?> tmp : listener) {
            if(tmp.currentlySelected) {
                if(tmp.mClass == EventLogClientFragment.class) {
                    return CLIENT_LOG_TAB_ACTIVE;
                }
                else if(tmp.mClass == EventLogGuiFragment.class) {
                    return GUI_LOG_TAB_ACTIVE;
                }
            }
        }
        return -1;
    }

    private void updateCurrentFragment() {
        for(EventLogActivityTabListener<?> tmp : listener) {
            if(tmp.currentlySelected) {
                if(tmp.mClass == EventLogClientFragment.class) {
                    ((EventLogClientFragment) tmp.mFragment).update();
                }
                else if(tmp.mClass == EventLogGuiFragment.class) {
                    ((EventLogGuiFragment) tmp.mFragment).update();
                }
                break;
            }
        }
    }

    private void onCopy() {
        try {
            ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
            ClipData clipData = ClipData.newPlainText("log", getLogDataAsString());
            clipboard.setPrimaryClip(clipData);
            Toast.makeText(getApplicationContext(), R.string.eventlog_copy_toast, Toast.LENGTH_SHORT).show();
        }
        catch(Exception e) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG, "onCopy failed");
            }
        }
    }

    private void onEmailTo() {
        try {
            String emailText = getLogDataAsString();

            Intent emailIntent = new Intent(android.content.Intent.ACTION_SEND);

            // Put together the email intent
            emailIntent.setType("plain/text");
            emailIntent.putExtra(android.content.Intent.EXTRA_SUBJECT, getString(R.string.eventlog_email_subject));

            emailIntent.putExtra(android.content.Intent.EXTRA_TEXT, emailText);

            // Send it off to the Activity-Chooser
            startActivity(Intent.createChooser(emailIntent, "Send mail..."));
        }
        catch(Exception e) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG, "onEmailTo failed");
            }
        }
    }

    // returns the content of the log as string
    // clientLog = true: client log
    // clientlog = false: gui log
    private String getLogDataAsString() {
        StringBuilder text = new StringBuilder();
        int type = getActiveLog();
        if(type == CLIENT_LOG_TAB_ACTIVE) {
            text.append(getString(R.string.eventlog_client_header)).append("\n\n");
            for(int index = 0; index < clientLogList.getCount(); index++) {
                text.append(clientLogListAdapter.getDate(index));
                text.append("|");
                text.append(clientLogListAdapter.getProject(index));
                text.append("|");
                text.append(clientLogListAdapter.getMessage(index));
                text.append("\n");
            }
        }
        else if(type == GUI_LOG_TAB_ACTIVE) {
            text.append(getString(R.string.eventlog_gui_header)).append("\n\n");
            for(String line : guiLogData) {
                text.append(line);
                text.append("\n");
            }
        }
        else if(Logging.WARNING) {
            Log.w(Logging.TAG, "EventLogActivity could not determine which log active.");
        }
        return text.toString();
    }
}
