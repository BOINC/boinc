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


public class HostInfo implements Parcelable {
    // all attributes are public for simple access
    /**
     * Local STANDARD time - UTC time (in seconds)
     */
    public int timezone;
    public String domain_name;
    public String ip_addr;
    public String host_cpid;
    public int p_ncpus;
    public String p_vendor;
    public String p_model;
    public String p_features;
    public double p_fpops;
    public double p_iops;
    public double p_membw;
    public String product_name;
    /**
     * When benchmarks were last run, or zero
     */
    public long p_calculated;
    /**
     * Total amount of memory in bytes
     */
    public double m_nbytes;
    public double m_cache;
    /**
     * Total amount of swap space in bytes
     */
    public double m_swap;
    /**
     * Total amount of disk in bytes
     */
    public double d_total;
    /**
     * Total amount of free disk in bytes
     */
    public double d_free;
    public String os_name;
    public String os_version;
    public String virtualbox_version = null;

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(timezone);
        dest.writeString(domain_name);
        dest.writeString(ip_addr);
        dest.writeString(host_cpid);
        dest.writeInt(p_ncpus);
        dest.writeString(p_vendor);
        dest.writeString(p_model);
        dest.writeString(p_features);
        dest.writeDouble(p_fpops);
        dest.writeDouble(p_iops);
        dest.writeDouble(p_membw);
        dest.writeString(product_name);
        dest.writeLong(p_calculated);
        dest.writeDouble(m_nbytes);
        dest.writeDouble(m_cache);
        dest.writeDouble(m_swap);
        dest.writeDouble(d_total);
        dest.writeDouble(d_free);
        dest.writeString(os_name);
        dest.writeString(os_version);
        dest.writeString(virtualbox_version);
    }

    public HostInfo() {
    }

    private HostInfo(Parcel in) {
        timezone = in.readInt();
        domain_name = in.readString();
        ip_addr = in.readString();
        host_cpid = in.readString();
        p_ncpus = in.readInt();
        p_vendor = in.readString();
        p_model = in.readString();
        p_features = in.readString();
        p_fpops = in.readDouble();
        p_iops = in.readDouble();
        p_membw = in.readDouble();
        product_name = in.readString();
        p_calculated = in.readLong();
        m_nbytes = in.readDouble();
        m_cache = in.readDouble();
        m_swap = in.readDouble();
        d_total = in.readDouble();
        d_free = in.readDouble();
        os_name = in.readString();
        os_version = in.readString();
        virtualbox_version = in.readString();

    }

    public static final Parcelable.Creator<HostInfo> CREATOR = new Parcelable.Creator<HostInfo>() {
        public HostInfo createFromParcel(Parcel in) {
            return new HostInfo(in);
        }

        public HostInfo[] newArray(int size) {
            return new HostInfo[size];
        }
    };
}
