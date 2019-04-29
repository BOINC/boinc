/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2013 University of California
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

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.*;
import android.support.v7.app.ActionBar.Tab;

public class EventLogActivityTabListener<T extends Fragment> implements ActionBar.TabListener {

    public Fragment mFragment;
    private final FragmentActivity mActivity;
    private final String mTag;
    public final Class<T> mClass;
    public Boolean currentlySelected = false;

    /**
     * Constructor used each time a new tab is created.
     *
     * @param activity The host Activity, used to instantiate the fragment
     * @param tag      The identifier tag for the fragment
     * @param clz      The fragment's Class, used to instantiate the fragment
     */
    public EventLogActivityTabListener(FragmentActivity activity, String tag, Class<T> clz) {
        mActivity = activity;
        mTag = tag;
        mClass = clz;

        // Check to see if we already have a fragment for this tab, probably
        // from a previously saved state.  If so, deactivate it, because our
        // initial state is that a tab isn't shown.
        mFragment = mActivity.getSupportFragmentManager().findFragmentByTag(mTag);
        if(mFragment != null && !mFragment.isDetached()) {
            FragmentTransaction ftd = mActivity.getSupportFragmentManager().beginTransaction();
            ftd.detach(mFragment);
            ftd.commit();
        }
    }

    /* The following are each of the ActionBar.TabListener callbacks */

    public void onTabSelected(Tab tab, FragmentTransaction ft) {
        // Check if the fragment is already initialized
        if(mFragment == null) {
            // If not, instantiate and add it to the activity
            mFragment = Fragment.instantiate(mActivity, mClass.getName());
            ft.add(android.R.id.content, mFragment, mTag);
        }
        else {
            // If it exists, simply attach it in order to show it
            ft.attach(mFragment);
        }
        currentlySelected = true;
    }

    public void onTabUnselected(Tab tab, FragmentTransaction ft) {
        if(mFragment != null) {
            // Detach the fragment, because another one is being attached
            ft.detach(mFragment);
        }
        currentlySelected = false;
    }

    public void onTabReselected(Tab tab, FragmentTransaction ft) {
    }
}
