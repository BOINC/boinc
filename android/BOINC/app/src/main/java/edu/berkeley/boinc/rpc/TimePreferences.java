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

import android.os.Parcel;
import android.os.Parcelable;

import lombok.EqualsAndHashCode;
import lombok.ToString;
import lombok.experimental.FieldNameConstants;

@EqualsAndHashCode
@FieldNameConstants
@ToString
public class TimePreferences implements Parcelable {
    public double start_hour, end_hour;

    @EqualsAndHashCode
    @ToString
    public static final class TimeSpan {
        public double start_hour;
        public double end_hour;
    }

    @FieldNameConstants.Exclude
    public TimeSpan[] week_prefs = new TimeSpan[7];

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
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

    TimePreferences() {
    }

    private TimePreferences(Parcel in) {
        double[] dArray = in.createDoubleArray();
        assert dArray != null;
        start_hour = dArray[0];
        end_hour = dArray[1];

        for (int i = 2; i <= 14; i += 2) {
            final int index = (i / 2) - 1;

            if (dArray[i] != Double.NEGATIVE_INFINITY) {
                week_prefs[index].start_hour = dArray[i];
                week_prefs[index].end_hour = dArray[i + 1];
            } else {
                week_prefs[index] = null;
            }
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
