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
package edu.berkeley.boinc.rpc

import android.os.Parcelable
import kotlinx.parcelize.Parcelize

@Parcelize
data class HostInfo(
    /**
     * Local STANDARD time - UTC time (in seconds)
     */
    var timezone: Int = 0,
    var domainName: String? = null,
    var ipAddress: String? = null,
    var hostCpid: String? = null,
    var noOfCPUs: Int = 0,
    var cpuVendor: String? = null,
    var cpuModel: String? = null,
    var cpuFeatures: String? = null,
    var cpuFloatingPointOps: Double = 0.0,
    var cpuIntegerOps: Double = 0.0,
    var cpuMembw: Double = 0.0,
    var productName: String? = null,
    /**
     * When benchmarks were last run, or zero
     */
    var cpuCalculated: Long = 0,
    /**
     * Total amount of memory in bytes
     */
    var memoryInBytes: Double = 0.0,
    var memoryCache: Double = 0.0,
    /**
     * Total amount of swap space in bytes
     */
    var memorySwap: Double = 0.0,
    /**
     * Total amount of disk space in bytes
     */
    var totalDiskSpace: Double = 0.0,
    /**
     * Total amount of free disk space in bytes
     */
    var freeDiskSpace: Double = 0.0,
    var osName: String? = null,
    var osVersion: String? = null,
    var virtualBoxVersion: String? = null
) : Parcelable {
    object Fields {
        const val TIMEZONE = "timezone"
        const val DOMAIN_NAME = "domain_name"
        const val IP_ADDR = "ip_addr"
        const val HOST_CPID = "host_cpid"
        const val P_NCPUS = "p_ncpus"
        const val P_VENDOR = "p_vendor"
        const val P_MODEL = "p_model"
        const val P_FEATURES = "p_features"
        const val P_FPOPS = "p_fpops"
        const val P_IOPS = "p_iops"
        const val P_MEMBW = "p_membw"
        const val PRODUCT_NAME = "product_name"
        const val P_CALCULATED = "p_calculated"
        const val M_NBYTES = "m_nbytes"
        const val M_CACHE = "m_cache"
        const val M_SWAP = "m_swap"
        const val D_TOTAL = "d_total"
        const val D_FREE = "d_free"
        const val OS_NAME = "os_name"
        const val OS_VERSION = "os_version"
        const val VIRTUALBOX_VERSION = "virtualbox_version"
    }
}
