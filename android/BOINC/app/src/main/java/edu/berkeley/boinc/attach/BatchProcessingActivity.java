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

package edu.berkeley.boinc.attach;

import java.util.ArrayList;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.*;
import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.attach.ProjectAttachService.ProjectAttachWrapper;

import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentStatePagerAdapter;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

public class BatchProcessingActivity extends FragmentActivity {

    private ProjectAttachService attachService = null;
    private boolean asIsBound = false;

    private static final int NUM_HINTS = 3; // number of available hint screens
    private ViewPager mPager; // pager widget, handles animation and horizontal swiping gestures
    private PagerAdapter mPagerAdapter; // provides content to pager
    private ArrayList<HintFragment> hints = new ArrayList<>(); // hint fragments

    //header
    private TextView hintTv;
    private ImageView hintIvRight;
    private ImageView hintIvLeft;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BatchProcessingActivity onCreate");
        }

        // setup layout
        setContentView(R.layout.attach_project_batch_processing_layout);

        hintTv = findViewById(R.id.hint_header_text);
        hintIvRight = findViewById(R.id.hint_header_image_right);
        hintIvLeft = findViewById(R.id.hint_header_image_left);

        // create hint fragments
        hints.add(HintFragment.newInstance(HintFragment.HINT_TYPE_CONTRIBUTION));
        hints.add(HintFragment.newInstance(HintFragment.HINT_TYPE_PROJECTWEBSITE));
        hints.add(HintFragment.newInstance(HintFragment.HINT_TYPE_PLATFORMS));

        // Instantiate a ViewPager and a PagerAdapter.
        mPager = findViewById(R.id.hint_container);
        mPagerAdapter = new HintPagerAdapter(getSupportFragmentManager());
        mPager.setAdapter(mPagerAdapter);
        mPager.setOnPageChangeListener(new OnPageChangeListener() {
            @Override
            public void onPageScrollStateChanged(int arg0) {
            }

            @Override
            public void onPageScrolled(int arg0, float arg1, int arg2) {
            }

            @Override
            public void onPageSelected(int arg0) {
                adaptHintHeader();
            }
        });
        adaptHintHeader();

        doBindService();
    }

    @Override
    protected void onDestroy() {
        if(Logging.VERBOSE) {
            Log.v(Logging.TAG, "BatchProcessingActivity onDestroy");
        }
        super.onDestroy();
        doUnbindService();
    }

    @Override
    public void onBackPressed() {
        if(mPager.getCurrentItem() == 0) {
            // If the user is currently looking at the first step, allow the system to handle the
            // Back button. This calls finish() on this activity and pops the back stack.
            super.onBackPressed();
        }
        else {
            // Otherwise, select the previous step.
            mPager.setCurrentItem(mPager.getCurrentItem() - 1);
        }
    }

    // triggered by continue button
    public void continueClicked(View v) {
        boolean conflicts = attachService.unresolvedConflicts();
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BatchProcessingActivity.continueClicked: conflicts? " + conflicts);
        }

        if(conflicts) {
            // conflicts occured, bring up resolution screen
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "AttachProjectAsyncTask: conflicts exists, open resolution activity...");
            }
            Intent intent = new Intent(BatchProcessingActivity.this, BatchConflictListActivity.class);
            intent.putExtra("conflicts", true);
            startActivity(intent);
        }
        else {
            // everything successful, go back to projects screen and clear history
            Intent intent = new Intent(this, BOINCActivity.class);
            // add flags to return to main activity and clearing all others and clear the back stack
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.putExtra("targetFragment", R.string.tab_projects); // make activity display projects fragment
            startActivity(intent);
        }
    }

    // triggered by share button
    public void shareClicked(View v) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BatchProcessingActivity.shareClicked.");
        }
        Intent intent = new Intent(android.content.Intent.ACTION_SEND);
        intent.setType("text/plain");
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);

        // Add data to the intent, the receiving app will decide what to do with it.
        intent.putExtra(Intent.EXTRA_SUBJECT, getString(R.string.social_invite_content_title));
        if(android.os.Build.MANUFACTURER.toUpperCase().equals("AMAZON")) {
            intent.putExtra(Intent.EXTRA_TEXT, String.format(getString(R.string.social_invite_content_body), android.os.Build.MANUFACTURER, getString(R.string.social_invite_content_url_amazon)));
        }
        else {
            intent.putExtra(Intent.EXTRA_TEXT, String.format(getString(R.string.social_invite_content_body), android.os.Build.MANUFACTURER, getString(R.string.social_invite_content_url_google)));
        }
        startActivity(Intent.createChooser(intent, getString(R.string.social_invite_intent_title)));
    }

    // adapts header text and icons when hint selection changes
    private void adaptHintHeader() {
        int position = mPager.getCurrentItem();
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BatchProcessingActivity.adaptHintHeader position: " + position);
        }
        String hintText = getString(R.string.attachproject_hints_header) + " " + (position + 1) + "/" + NUM_HINTS;
        hintTv.setText(hintText);
        int leftVisibility = View.VISIBLE;
        int rightVisibility = View.VISIBLE;
        if(position == 0) {
            // first element reached
            leftVisibility = View.GONE;
        }
        else if(position == NUM_HINTS - 1) {
            // last element reached
            rightVisibility = View.GONE;
        }
        hintIvLeft.setVisibility(leftVisibility);
        hintIvRight.setVisibility(rightVisibility);
    }

    // previous image in hint header clicked
    public void previousHintClicked(View view) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BatchProcessingActivity.previousHintClicked.");
        }
        mPager.setCurrentItem(mPager.getCurrentItem() - 1);
    }

    // previous image in hint header clicked
    public void nextHintClicked(View view) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "BatchProcessingActivity.nextHintClicked.");
        }
        mPager.setCurrentItem(mPager.getCurrentItem() + 1);
    }

    private ServiceConnection mASConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            attachService = ((ProjectAttachService.LocalBinder) service).getService();
            asIsBound = true;

            // start attaching projects
            new AttachProjectAsyncTask().execute();
        }

        public void onServiceDisconnected(ComponentName className) {
            // This should not happen
            attachService = null;
            asIsBound = false;
        }
    };

    private void doBindService() {
        // bind to attach service
        bindService(new Intent(this, ProjectAttachService.class), mASConnection, Service.BIND_AUTO_CREATE);
    }

    private void doUnbindService() {
        if(asIsBound) {
            // Detach existing connection.
            unbindService(mASConnection);
            asIsBound = false;
        }
    }

    private class AttachProjectAsyncTask extends AsyncTask<Void, String, Void> {

        @Override
        protected void onPreExecute() {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "AttachProjectAsyncTask: " + attachService.getNumberSelectedProjects() +
                                   " projects to attach....");
            }
            ((TextView) findViewById(R.id.attach_status_text)).setText(getString(R.string.attachproject_login_loading)); // shown while project configs are loaded
            super.onPreExecute();
        }

        @Override
        protected Void doInBackground(Void... arg0) {
            // wait until service is ready
            while(!attachService.projectConfigRetrievalFinished) {
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "AttachProjectAsyncTask: project config retrieval has not finished yet, wait...");
                }
                try {
                    Thread.sleep(1000);
                }
                catch(Exception ignored) {
                }
            }
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "AttachProjectAsyncTask: project config retrieval finished, continue with attach.");
            }
            // attach projects, one at a time
            ArrayList<ProjectAttachWrapper> selectedProjects = attachService.getSelectedProjects();
            for(ProjectAttachWrapper selectedProject : selectedProjects) {
                if(selectedProject.result != ProjectAttachWrapper.RESULT_READY) {
                    continue; // skip already tried projects in batch processing
                }
                publishProgress(selectedProject.info.name);
                int conflict = selectedProject.lookupAndAttach(false);
                if(conflict != ProjectAttachWrapper.RESULT_SUCCESS) {
                    if(Logging.ERROR) {
                        Log.e(Logging.TAG, "AttachProjectAsyncTask attach returned conflict: " + conflict);
                    }
                }
            }
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "AttachProjectAsyncTask: finsihed.");
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(String... values) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "AttachProjectAsyncTask: trying: " + values[0]);
            }
            ((TextView) findViewById(R.id.attach_status_text)).setText(
                    getString(R.string.attachproject_working_attaching) + " " + values[0]);
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPostExecute(Void result) {
            findViewById(R.id.attach_status_ongoing_wrapper).setVisibility(View.GONE);
            findViewById(R.id.continue_button).setVisibility(View.VISIBLE);
            findViewById(R.id.share_button).setVisibility(View.VISIBLE);
            super.onPostExecute(result);
        }
    }

    private class HintPagerAdapter extends FragmentStatePagerAdapter {

        public HintPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        @Override
        public Fragment getItem(int position) {
            return hints.get(position);
        }

        @Override
        public int getCount() {
            return NUM_HINTS;
        }
    }
}
