/*******************************************************************************
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
 ******************************************************************************/

package edu.berkeley.boinc.rpc;


public class HostInfo {
	// all attributes are public for simple access
	/** Local STANDARD time - UTC time (in seconds) */
	public int    timezone;
	public String domain_name;
	public String ip_addr;
	public String host_cpid;
	public int    p_ncpus;
	public String p_vendor;
	public String p_model;
	public String p_features;
	public double p_fpops;
	public double p_iops;
	public double p_membw;
	public String product_name;
	/** When benchmarks were last run, or zero */
	public long   p_calculated;
	/** Total amount of memory in bytes */
	public double m_nbytes;
	public double m_cache;
	/** Total amount of swap space in bytes */
	public double m_swap;
	/** Total amount of disk in bytes */
	public double d_total;
	/** Total amount of free disk in bytes */
	public double d_free;
	public String os_name;
	public String os_version;
	public String virtualbox_version = null;
}
