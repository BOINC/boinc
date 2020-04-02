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

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import android.util.Log;
import android.util.Xml;

import edu.berkeley.boinc.utils.Logging;

public class GlobalPreferencesParser extends BaseParser {
    static final String GLOBAL_PREFERENCES_TAG = "global_preferences";
    static final String DAY_PREFS_TAG = "day_prefs";
    static final String DAY_OF_WEEK_TAG = "day_of_week";
    static final String NET_START_HOUR_TAG = "net_start_hour";
    static final String NET_END_HOUR_TAG = "net_end_hour";

    private GlobalPreferences mPreferences = null;

    private boolean mInsideDayPrefs = false;
    private int mDayOfWeek = 0;
    private TimeSpan mTempCpuTimeSpan = null;
    private TimeSpan mTempNetTimeSpan = null;

    GlobalPreferences getGlobalPreferences() {
        return mPreferences;
    }

    public static GlobalPreferences parse(String rpcResult) {
        try {
            GlobalPreferencesParser parser = new GlobalPreferencesParser();
            Xml.parse(rpcResult, parser);
            return parser.getGlobalPreferences();
        }
        catch(SAXException e) {
            return null;
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(GLOBAL_PREFERENCES_TAG)) {
            mPreferences = new GlobalPreferences();
        }
        else if(localName.equalsIgnoreCase(DAY_PREFS_TAG)) {
            mInsideDayPrefs = true;
        }
        else {
            // Another element, hopefully primitive and not constructor
            // (although unknown constructor does not hurt, because there will be primitive start anyway)
            mElementStarted = true;
            mCurrentElement.setLength(0);
        }
    }

    @Override
    public void endElement(String uri, String localName, String qName) throws SAXException {
        super.endElement(uri, localName, qName);
        try {
            if(mPreferences != null) {
                // we are inside <global_preferences>
                // Closing tag of <global_preferences> - nothing to do at the moment
                if(!localName.equalsIgnoreCase(GLOBAL_PREFERENCES_TAG) &&
                   localName.equalsIgnoreCase(DAY_PREFS_TAG)) {
                    // closing <day_prefs>
                    if(mDayOfWeek >= 0 && mDayOfWeek <= 6) {
                        mPreferences.getCpuTimes().getWeekPrefs()[mDayOfWeek] = mTempCpuTimeSpan;
                        mPreferences.getNetTimes().getWeekPrefs()[mDayOfWeek] = mTempNetTimeSpan;
                    }

                    mTempCpuTimeSpan = null;
                    mTempNetTimeSpan = null;
                    mInsideDayPrefs = false;
                }
                else if(mInsideDayPrefs) {
                    trimEnd();
                    if(localName.equalsIgnoreCase(DAY_OF_WEEK_TAG)) {
                        mDayOfWeek = Integer.parseInt(mCurrentElement.toString());
                    }
                    else if(localName.equalsIgnoreCase(TimePreferences.Fields.START_HOUR)) {
                        if(mTempCpuTimeSpan == null) {
                            mTempCpuTimeSpan = new TimeSpan();
                        }
                        mTempCpuTimeSpan.setStartHour(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(TimePreferences.Fields.END_HOUR)) {
                        if(mTempCpuTimeSpan == null) {
                            mTempCpuTimeSpan = new TimeSpan();
                        }
                        mTempCpuTimeSpan.setEndHour(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(NET_START_HOUR_TAG)) {
                        if(mTempNetTimeSpan == null) {
                            mTempNetTimeSpan = new TimeSpan();
                        }
                        mTempNetTimeSpan.setStartHour(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(NET_END_HOUR_TAG)) {
                        if(mTempNetTimeSpan == null) {
                            mTempNetTimeSpan = new TimeSpan();
                        }
                        mTempNetTimeSpan.setEndHour(Double.parseDouble(mCurrentElement.toString()));
                    }
                }
                else {
                    // Not the closing tag - we decode possible inner tags
                    trimEnd();
                    if(localName.equalsIgnoreCase(GlobalPreferences.Fields.RUN_ON_BATTERIES)) {
                        mPreferences.setRunOnBatteryPower(Integer.parseInt(mCurrentElement.toString()) != 0);
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.BATTERY_CHARGE_MIN_PCT)) {
                        mPreferences.setBatteryChargeMinPct(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.BATTERY_MAX_TEMPERATURE)) {
                        mPreferences.setBatteryMaxTemperature(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.RUN_GPU_IF_USER_ACTIVE)) {
                        mPreferences.setRunGpuIfUserActive(Integer.parseInt(mCurrentElement.toString()) != 0);
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.RUN_IF_USER_ACTIVE)) {
                        mPreferences.setRunIfUserActive(Integer.parseInt(mCurrentElement.toString()) != 0);
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.IDLE_TIME_TO_RUN)) {
                        mPreferences.setIdleTimeToRun(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.SUSPEND_CPU_USAGE)) {
                        mPreferences.setSuspendCpuUsage(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.LEAVE_APPS_IN_MEMORY)) {
                        mPreferences.setLeaveAppsInMemory(Integer.parseInt(mCurrentElement.toString()) != 0);
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.DONT_VERIFY_IMAGES)) {
                        mPreferences.setDoNotVerifyImages(Integer.parseInt(mCurrentElement.toString()) != 0);
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.WORK_BUF_MIN_DAYS)) {
                        mPreferences.setWorkBufMinDays(
                                Math.max(0.00001, Double.parseDouble(mCurrentElement.toString())));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.WORK_BUF_ADDITIONAL_DAYS)) {
                        mPreferences.setWorkBufAdditionalDays(
                                Math.max(0.0, Double.parseDouble(mCurrentElement.toString())));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.MAX_NCPUS_PCT)) {
                        mPreferences.setMaxNoOfCPUsPct(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.CPU_SCHEDULING_PERIOD_MINUTES)) {
                        double value = Double.parseDouble(mCurrentElement.toString());
                        if (value < 0.00001) {
                            value = 60;
                        }
                        mPreferences.setCpuSchedulingPeriodMinutes(value);
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.DISK_INTERVAL)) {
                        mPreferences.setDiskInterval(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.DISK_MAX_USED_GB)) {
                        mPreferences.setDiskMaxUsedGB(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.DISK_MAX_USED_PCT)) {
                        mPreferences.setDiskMaxUsedPct(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.DISK_MIN_FREE_GB)) {
                        mPreferences.setDiskMinFreeGB(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.RAM_MAX_USED_BUSY_FRAC)) {
                        mPreferences.setRamMaxUsedBusyFrac(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.RAM_MAX_USED_IDLE_FRAC)) {
                        mPreferences.setRamMaxUsedIdleFrac(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.MAX_BYTES_SEC_UP)) {
                        mPreferences.setMaxBytesSecUp(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.MAX_BYTES_SEC_DOWN)) {
                        mPreferences.setMaxBytesSecDown(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.CPU_USAGE_LIMIT)) {
                        mPreferences.setCpuUsageLimit(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.DAILY_XFER_PERIOD_MB)) {
                        mPreferences.setDailyTransferLimitMB(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.DAILY_XFER_PERIOD_DAYS)) {
                        mPreferences.setDailyTransferPeriodDays(Integer.parseInt(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(TimePreferences.Fields.START_HOUR)) {
                        mPreferences.getCpuTimes().setStartHour(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(TimePreferences.Fields.END_HOUR)) {
                        mPreferences.getCpuTimes().setEndHour(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(NET_START_HOUR_TAG)) {
                        mPreferences.getNetTimes().setStartHour(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(NET_END_HOUR_TAG)) {
                        mPreferences.getNetTimes().setEndHour(Double.parseDouble(mCurrentElement.toString()));
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.OVERRIDE_FILE_PRESENT)) {
                        mPreferences.setOverrideFilePresent(Integer.parseInt(mCurrentElement.toString()) != 0);
                    }
                    else if(localName.equalsIgnoreCase(GlobalPreferences.Fields.NETWORK_WIFI_ONLY)) {
                        mPreferences.setNetworkWiFiOnly(Integer.parseInt(mCurrentElement.toString()) != 0);
                    }
                }
            }
        }
        catch(NumberFormatException e) {
            if(Logging.ERROR.equals(Boolean.TRUE)) {
                Log.e(Logging.TAG, "GlobalPreferencesParser.endElement error: ", e);
            }
        }
        mElementStarted = false;
    }
}
