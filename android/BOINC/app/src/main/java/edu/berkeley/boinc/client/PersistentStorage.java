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
package edu.berkeley.boinc.client;

import android.content.Context;
import android.content.SharedPreferences;

/**
 * This class wraps persistent key value pairs.
 * Similar technique to AppPrefs, but with a non-preference incentive.
 */
public class PersistentStorage {

    private final String STORE = "Store";
    private SharedPreferences store;

    public PersistentStorage(Context ctx) {
        this.store = ctx.getSharedPreferences(STORE, 0);
    }

    public double getLastNotifiedNoticeArrivalTime() {
        long defaultValue = 0;
        return Double.longBitsToDouble(store.getLong("lastNotifiedNoticeArrivalTime", Double.doubleToRawLongBits(defaultValue)));
    }

    public void setLastNotifiedNoticeArrivalTime(double arrivalTime) {
        SharedPreferences.Editor editor = store.edit();
        editor.putLong("lastNotifiedNoticeArrivalTime", Double.doubleToRawLongBits(arrivalTime));
        editor.apply();
    }

    public String getLastEmailAddress() {
        return store.getString("lastEmailAddress", "");
    }

    public void setLastEmailAddress(String email) {
        SharedPreferences.Editor editor = store.edit();
        editor.putString("lastEmailAddress", email);
        editor.apply();
    }

    public String getLastUserName() {
        return store.getString("lastUserName", "");
    }

    public void setLastUserName(String name) {
        SharedPreferences.Editor editor = store.edit();
        editor.putString("lastUserName", name);
        editor.apply();
    }
}
