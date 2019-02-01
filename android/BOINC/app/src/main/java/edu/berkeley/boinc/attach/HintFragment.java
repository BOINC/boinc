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

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.*;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

public class HintFragment extends Fragment {

    private int type;
    public static final int HINT_TYPE_CONTRIBUTION = 1;
    public static final int HINT_TYPE_PROJECTWEBSITE = 2;
    public static final int HINT_TYPE_PLATFORMS = 3;

    static HintFragment newInstance(int hintType) {
        HintFragment frag = new HintFragment();
        frag.type = hintType;
        return frag;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "HintFragment onCreateView for hint type: " + type);
        }
        View v = null;
        switch(type) {
            case HINT_TYPE_CONTRIBUTION:
                v = inflater.inflate(R.layout.attach_project_hint_contribution_layout, container, false);
                break;
            case HINT_TYPE_PROJECTWEBSITE:
                v = inflater.inflate(R.layout.attach_project_hint_projectwebsite_layout, container, false);
                break;
            case HINT_TYPE_PLATFORMS:
                v = inflater.inflate(R.layout.attach_project_hint_platforms_layout, container, false);
                break;
        }
        return v;
    }
}
