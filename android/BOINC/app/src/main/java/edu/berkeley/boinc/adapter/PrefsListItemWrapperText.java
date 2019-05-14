/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2016 University of California
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

package edu.berkeley.boinc.adapter;

import android.content.Context;
import android.util.Log;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.Logging;

public class PrefsListItemWrapperText extends PrefsListItemWrapper {
    public String status;

    public PrefsListItemWrapperText(Context ctx, Integer ID, Integer categoryID, String status) {
        super(ctx, ID, categoryID);
        this.status = status;
        this.dialogButtonType = DialogButtonType.TEXT;
        mapStrings(ID);
    }

    private void mapStrings(Integer id) {
        switch(id) {
            case R.string.prefs_general_device_name_header:
                description = "";
                break;
            default:
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "PrefsListItemWrapperText map failed!");
                }
        }
    }
}
