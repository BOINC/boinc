/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2020 University of California
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

import com.google.common.testing.EqualsTester
import org.junit.jupiter.api.Test

class HostInfoTest {
    @Test
    fun `Test equals() and hashCode()`() {
        EqualsTester().addEqualityGroup(HostInfo(), HostInfo())
                .addEqualityGroup(HostInfo(timezone = 1))
                .addEqualityGroup(HostInfo(domainName = "Domain name"))
                .addEqualityGroup(HostInfo(ipAddress = "IP address"))
                .addEqualityGroup(HostInfo(hostCpid = "Host CPID"))
                .addEqualityGroup(HostInfo(noOfCPUs = 1))
                .addEqualityGroup(HostInfo(cpuVendor = "Vendor"))
                .addEqualityGroup(HostInfo(cpuModel = "Model"))
                .addEqualityGroup(HostInfo(cpuFeatures = "Features"))
                .addEqualityGroup(HostInfo(cpuFloatingPointOps = 1.0))
                .addEqualityGroup(HostInfo(cpuIntegerOps = 1.0))
                .addEqualityGroup(HostInfo(cpuMembw = 1.0))
                .addEqualityGroup(HostInfo(productName = "Product name"))
                .addEqualityGroup(HostInfo(cpuCalculated = 1))
                .addEqualityGroup(HostInfo(memoryInBytes = 1.0))
                .addEqualityGroup(HostInfo(memoryCache = 1.0))
                .addEqualityGroup(HostInfo(memorySwap = 1.0))
                .addEqualityGroup(HostInfo(totalDiskSpace = 1.0))
                .addEqualityGroup(HostInfo(freeDiskSpace = 1.0))
                .addEqualityGroup(HostInfo(osName = "OS name"))
                .addEqualityGroup(HostInfo(osVersion = "OS version"))
                .addEqualityGroup(HostInfo(virtualBoxVersion = "VirtualBox version"))
                .testEquals()
    }
}
