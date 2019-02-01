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
package edu.berkeley.boinc.rpc;

import java.util.Locale;

import android.os.Parcel;
import android.os.Parcelable;

public class TimePreferences implements Parcelable {
    public double start_hour, end_hour;

    private static final String hourToString(double value) {
        int hour = (int) Math.floor(value);
        int minute = (int) Math.round((value - (double) hour) * 60.0);
        minute = Math.min(59, minute);
        return String.format(Locale.US, "%02d:%02d", hour, minute);
    }

    public static final class TimeSpan {
        public double start_hour;
        public double end_hour;

        public String startHourString() {
            return hourToString(start_hour);
        }

        public String endHourString() {
            return hourToString(end_hour);
        }
    }

    public TimeSpan[] week_prefs = new TimeSpan[7];

    public String startHourString() {
        return hourToString(start_hour);
    }

    public String endHourString() {
        return hourToString(end_hour);
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        // TODO Auto-generated method stub
        dest.writeDoubleArray(new double[]{
                start_hour, end_hour,
                week_prefs[0] != null ? week_prefs[0].start_hour : Double.NEGATIVE_INFINITY,
                week_prefs[0] != null ? week_prefs[0].end_hour : Double.NEGATIVE_INFINITY,
                week_prefs[1] != null ? week_prefs[1].start_hour : Double.NEGATIVE_INFINITY,
                week_prefs[1] != null ? week_prefs[1].end_hour : Double.NEGATIVE_INFINITY,
                week_prefs[2] != null ? week_prefs[2].start_hour : Double.NEGATIVE_INFINITY,
                week_prefs[2] != null ? week_prefs[2].end_hour : Double.NEGATIVE_INFINITY,
                week_prefs[3] != null ? week_prefs[3].start_hour : Double.NEGATIVE_INFINITY,
                week_prefs[3] != null ? week_prefs[3].end_hour : Double.NEGATIVE_INFINITY,
                week_prefs[4] != null ? week_prefs[4].start_hour : Double.NEGATIVE_INFINITY,
                week_prefs[4] != null ? week_prefs[4].end_hour : Double.NEGATIVE_INFINITY,
                week_prefs[5] != null ? week_prefs[5].start_hour : Double.NEGATIVE_INFINITY,
                week_prefs[5] != null ? week_prefs[5].end_hour : Double.NEGATIVE_INFINITY,
                week_prefs[6] != null ? week_prefs[6].start_hour : Double.NEGATIVE_INFINITY,
                week_prefs[6] != null ? week_prefs[6].end_hour : Double.NEGATIVE_INFINITY
        });
    }

    public TimePreferences() {
    }

    private TimePreferences(Parcel in) {
        double[] dArray = in.createDoubleArray();
        start_hour = dArray[0];
        end_hour = dArray[1];
        if(dArray[2] != Double.NEGATIVE_INFINITY) {
            week_prefs[0].start_hour = dArray[2];
            week_prefs[0].end_hour = dArray[3];
        }
        else {
            week_prefs[0] = null;
        }

        if(dArray[4] != Double.NEGATIVE_INFINITY) {
            week_prefs[1].start_hour = dArray[4];
            week_prefs[1].end_hour = dArray[5];
        }
        else {
            week_prefs[1] = null;
        }

        if(dArray[6] != Double.NEGATIVE_INFINITY) {
            week_prefs[2].start_hour = dArray[6];
            week_prefs[2].end_hour = dArray[7];
        }
        else {
            week_prefs[2] = null;
        }

        if(dArray[8] != Double.NEGATIVE_INFINITY) {
            week_prefs[3].start_hour = dArray[8];
            week_prefs[3].end_hour = dArray[9];
        }
        else {
            week_prefs[3] = null;
        }

        if(dArray[10] != Double.NEGATIVE_INFINITY) {
            week_prefs[4].start_hour = dArray[10];
            week_prefs[4].end_hour = dArray[11];
        }
        else {
            week_prefs[4] = null;
        }

        if(dArray[12] != Double.NEGATIVE_INFINITY) {
            week_prefs[5].start_hour = dArray[12];
            week_prefs[5].end_hour = dArray[13];
        }
        else {
            week_prefs[5] = null;
        }

        if(dArray[14] != Double.NEGATIVE_INFINITY) {
            week_prefs[6].start_hour = dArray[14];
            week_prefs[6].end_hour = dArray[15];
        }
        else {
            week_prefs[6] = null;
        }
    }

    public static final Parcelable.Creator<TimePreferences> CREATOR = new Parcelable.Creator<TimePreferences>() {
        @Override
        public TimePreferences createFromParcel(Parcel in) {
            return new TimePreferences(in);
        }

        public TimePreferences[] newArray(int size) {
            return new TimePreferences[size];
        }
    };
}
