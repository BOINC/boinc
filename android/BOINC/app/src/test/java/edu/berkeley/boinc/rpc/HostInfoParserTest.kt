/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2021 University of California
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

import android.util.Log
import android.util.Xml
import edu.berkeley.boinc.rpc.HostInfoParser.Companion.parse
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.ArgumentMatchers
import org.powermock.api.mockito.PowerMockito
import org.powermock.core.classloader.annotations.PrepareForTest
import org.powermock.modules.junit4.PowerMockRunner
import org.xml.sax.ContentHandler
import org.xml.sax.SAXException

@RunWith(PowerMockRunner::class)
@PrepareForTest(Log::class, Xml::class)
class HostInfoParserTest {
    private lateinit var hostInfoParser: HostInfoParser
    private lateinit var expected: HostInfo
    @Before
    fun setUp() {
        hostInfoParser = HostInfoParser()
        expected = HostInfo()
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    fun `When Rpc string is null then expect UninitializedPropertyAccessException`() {
        PowerMockito.mockStatic(Xml::class.java)
        parse(null)
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    fun `When Rpc string is empty then expect UninitializedPropertyAccessException`() {
        PowerMockito.mockStatic(Xml::class.java)
        parse("")
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown then expect null`() {
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertNull(parse(""))
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect element started`() {
        hostInfoParser.startElement(null, "", null, null)
        Assert.assertTrue(hostInfoParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is host_info tag then expect element not started`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        Assert.assertFalse(hostInfoParser.mElementStarted)
    }

    @Test(expected = UninitializedPropertyAccessException::class)
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect UninitializedPropertyAccessException`() {
        hostInfoParser.startElement(null, "", null, null)
        hostInfoParser.hostInfo
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is host_info tag then expect default HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has timezone then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.TIMEZONE, null, null)
        hostInfoParser.characters("1".toCharArray(), 0, 1)
        hostInfoParser.endElement(null, HostInfo.Fields.TIMEZONE, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.timezone = 1
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has domain_name and invalid timezone then expect HostInfo with only domain name`() {
        PowerMockito.mockStatic(Log::class.java)
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.DOMAIN_NAME, null, null)
        hostInfoParser.characters(DOMAIN_NAME.toCharArray(), 0, 11)
        hostInfoParser.endElement(null, HostInfo.Fields.DOMAIN_NAME, null)
        hostInfoParser.startElement(null, HostInfo.Fields.TIMEZONE, null, null)
        hostInfoParser.characters("One".toCharArray(), 0, 3)
        hostInfoParser.endElement(null, HostInfo.Fields.TIMEZONE, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.domainName = DOMAIN_NAME
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has domain_name then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.DOMAIN_NAME, null, null)
        hostInfoParser.characters(DOMAIN_NAME.toCharArray(), 0, 11)
        hostInfoParser.endElement(null, HostInfo.Fields.DOMAIN_NAME, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.domainName = DOMAIN_NAME
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has ip_address then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.IP_ADDR, null, null)
        hostInfoParser.characters(IP_ADDRESS.toCharArray(), 0, IP_ADDRESS.length)
        hostInfoParser.endElement(null, HostInfo.Fields.IP_ADDR, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.ipAddress = IP_ADDRESS
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has host_cpid then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.HOST_CPID, null, null)
        hostInfoParser.characters(HOST_CPID.toCharArray(), 0, HOST_CPID.length)
        hostInfoParser.endElement(null, HostInfo.Fields.HOST_CPID, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.hostCpid = HOST_CPID
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has no_of_cpus then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_NCPUS, null, null)
        hostInfoParser.characters("1".toCharArray(), 0, 1)
        hostInfoParser.endElement(null, HostInfo.Fields.P_NCPUS, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.noOfCPUs = 1
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has vendor then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_VENDOR, null, null)
        hostInfoParser.characters(VENDOR.toCharArray(), 0, VENDOR.length)
        hostInfoParser.endElement(null, HostInfo.Fields.P_VENDOR, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.cpuVendor = VENDOR
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has model then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_MODEL, null, null)
        hostInfoParser.characters(MODEL.toCharArray(), 0, MODEL.length)
        hostInfoParser.endElement(null, HostInfo.Fields.P_MODEL, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.cpuModel = MODEL
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has features then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_FEATURES, null, null)
        hostInfoParser.characters(FEATURES.toCharArray(), 0, FEATURES.length)
        hostInfoParser.endElement(null, HostInfo.Fields.P_FEATURES, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.cpuFeatures = FEATURES
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has floating_point_ops then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_FPOPS, null, null)
        hostInfoParser.characters("1.5".toCharArray(), 0, 3)
        hostInfoParser.endElement(null, HostInfo.Fields.P_FPOPS, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.cpuFloatingPointOps = 1.5
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has integer_ops then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_IOPS, null, null)
        hostInfoParser.characters("1.5".toCharArray(), 0, 3)
        hostInfoParser.endElement(null, HostInfo.Fields.P_IOPS, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.cpuIntegerOps = 1.5
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has membw then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_MEMBW, null, null)
        hostInfoParser.characters("1.5".toCharArray(), 0, 3)
        hostInfoParser.endElement(null, HostInfo.Fields.P_MEMBW, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.cpuMembw = 1.5
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has calculated then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_CALCULATED, null, null)
        hostInfoParser.characters("0".toCharArray(), 0, 1)
        hostInfoParser.endElement(null, HostInfo.Fields.P_CALCULATED, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.cpuCalculated = 0L
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has product_name then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.PRODUCT_NAME, null, null)
        hostInfoParser.characters(PRODUCT_NAME.toCharArray(), 0, PRODUCT_NAME.length)
        hostInfoParser.endElement(null, HostInfo.Fields.PRODUCT_NAME, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.productName = PRODUCT_NAME
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has mem_in_bytes then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.M_NBYTES, null, null)
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length)
        hostInfoParser.endElement(null, HostInfo.Fields.M_NBYTES, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.memoryInBytes = 1073741824.0
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has mem_cache then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.M_CACHE, null, null)
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length)
        hostInfoParser.endElement(null, HostInfo.Fields.M_CACHE, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.memoryCache = 1073741824.0
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has mem_swap then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.M_SWAP, null, null)
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length)
        hostInfoParser.endElement(null, HostInfo.Fields.M_SWAP, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.memorySwap = 1073741824.0
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has disk_total then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.D_TOTAL, null, null)
        hostInfoParser.characters(TOTAL_SPACE_STR.toCharArray(), 0, TOTAL_SPACE_STR.length)
        hostInfoParser.endElement(null, HostInfo.Fields.D_TOTAL, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.totalDiskSpace = TOTAL_SPACE.toDouble()
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has disk_free then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.D_FREE, null, null)
        hostInfoParser.characters(FREE_SPACE_STR.toCharArray(), 0, FREE_SPACE_STR.length)
        hostInfoParser.endElement(null, HostInfo.Fields.D_FREE, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.freeDiskSpace = FREE_SPACE.toDouble()
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has os_name then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.OS_NAME, null, null)
        hostInfoParser.characters(OS_NAME.toCharArray(), 0, OS_NAME.length)
        hostInfoParser.endElement(null, HostInfo.Fields.OS_NAME, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.osName = OS_NAME
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has os_version then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.OS_VERSION, null, null)
        hostInfoParser.characters(OS_VERSION.toCharArray(), 0, OS_VERSION.length)
        hostInfoParser.endElement(null, HostInfo.Fields.OS_VERSION, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.osVersion = OS_VERSION
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has virtualbox_version then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.VIRTUALBOX_VERSION, null, null)
        hostInfoParser.characters(VIRTUALBOX_VERSION.toCharArray(), 0, VIRTUALBOX_VERSION.length)
        hostInfoParser.endElement(null, HostInfo.Fields.VIRTUALBOX_VERSION, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.virtualBoxVersion = VIRTUALBOX_VERSION
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml HostInfo has all attributes then expect matching HostInfo`() {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        hostInfoParser.startElement(null, HostInfo.Fields.TIMEZONE, null, null)
        hostInfoParser.characters("1".toCharArray(), 0, 1)
        hostInfoParser.endElement(null, HostInfo.Fields.TIMEZONE, null)
        hostInfoParser.startElement(null, HostInfo.Fields.DOMAIN_NAME, null, null)
        hostInfoParser.characters(DOMAIN_NAME.toCharArray(), 0, 11)
        hostInfoParser.endElement(null, HostInfo.Fields.DOMAIN_NAME, null)
        hostInfoParser.startElement(null, HostInfo.Fields.IP_ADDR, null, null)
        hostInfoParser.characters(IP_ADDRESS.toCharArray(), 0, IP_ADDRESS.length)
        hostInfoParser.endElement(null, HostInfo.Fields.IP_ADDR, null)
        hostInfoParser.startElement(null, HostInfo.Fields.HOST_CPID, null, null)
        hostInfoParser.characters(HOST_CPID.toCharArray(), 0, HOST_CPID.length)
        hostInfoParser.endElement(null, HostInfo.Fields.HOST_CPID, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_NCPUS, null, null)
        hostInfoParser.characters("1".toCharArray(), 0, 1)
        hostInfoParser.endElement(null, HostInfo.Fields.P_NCPUS, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_VENDOR, null, null)
        hostInfoParser.characters(VENDOR.toCharArray(), 0, VENDOR.length)
        hostInfoParser.endElement(null, HostInfo.Fields.P_VENDOR, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_MODEL, null, null)
        hostInfoParser.characters(MODEL.toCharArray(), 0, MODEL.length)
        hostInfoParser.endElement(null, HostInfo.Fields.P_MODEL, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_FEATURES, null, null)
        hostInfoParser.characters(FEATURES.toCharArray(), 0, FEATURES.length)
        hostInfoParser.endElement(null, HostInfo.Fields.P_FEATURES, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_FPOPS, null, null)
        hostInfoParser.characters("1.5".toCharArray(), 0, 3)
        hostInfoParser.endElement(null, HostInfo.Fields.P_FPOPS, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_IOPS, null, null)
        hostInfoParser.characters("1.5".toCharArray(), 0, 3)
        hostInfoParser.endElement(null, HostInfo.Fields.P_IOPS, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_MEMBW, null, null)
        hostInfoParser.characters("1.5".toCharArray(), 0, 3)
        hostInfoParser.endElement(null, HostInfo.Fields.P_MEMBW, null)
        hostInfoParser.startElement(null, HostInfo.Fields.P_CALCULATED, null, null)
        hostInfoParser.characters("0".toCharArray(), 0, 1)
        hostInfoParser.endElement(null, HostInfo.Fields.P_CALCULATED, null)
        hostInfoParser.startElement(null, HostInfo.Fields.PRODUCT_NAME, null, null)
        hostInfoParser.characters(PRODUCT_NAME.toCharArray(), 0, PRODUCT_NAME.length)
        hostInfoParser.endElement(null, HostInfo.Fields.PRODUCT_NAME, null)
        hostInfoParser.startElement(null, HostInfo.Fields.M_NBYTES, null, null)
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length)
        hostInfoParser.endElement(null, HostInfo.Fields.M_NBYTES, null)
        hostInfoParser.startElement(null, HostInfo.Fields.M_CACHE, null, null)
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length)
        hostInfoParser.endElement(null, HostInfo.Fields.M_CACHE, null)
        hostInfoParser.startElement(null, HostInfo.Fields.M_SWAP, null, null)
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length)
        hostInfoParser.endElement(null, HostInfo.Fields.M_SWAP, null)
        hostInfoParser.startElement(null, HostInfo.Fields.D_TOTAL, null, null)
        hostInfoParser.characters(TOTAL_SPACE_STR.toCharArray(), 0, TOTAL_SPACE_STR.length)
        hostInfoParser.endElement(null, HostInfo.Fields.D_TOTAL, null)
        hostInfoParser.startElement(null, HostInfo.Fields.D_FREE, null, null)
        hostInfoParser.characters(FREE_SPACE_STR.toCharArray(), 0, FREE_SPACE_STR.length)
        hostInfoParser.endElement(null, HostInfo.Fields.D_FREE, null)
        hostInfoParser.startElement(null, HostInfo.Fields.OS_NAME, null, null)
        hostInfoParser.characters(OS_NAME.toCharArray(), 0, OS_NAME.length)
        hostInfoParser.endElement(null, HostInfo.Fields.OS_NAME, null)
        hostInfoParser.startElement(null, HostInfo.Fields.OS_VERSION, null, null)
        hostInfoParser.characters(OS_VERSION.toCharArray(), 0, OS_VERSION.length)
        hostInfoParser.endElement(null, HostInfo.Fields.OS_VERSION, null)
        hostInfoParser.startElement(null, HostInfo.Fields.VIRTUALBOX_VERSION, null, null)
        hostInfoParser.characters(VIRTUALBOX_VERSION.toCharArray(), 0, VIRTUALBOX_VERSION.length)
        hostInfoParser.endElement(null, HostInfo.Fields.VIRTUALBOX_VERSION, null)
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        expected.timezone = 1
        expected.domainName = DOMAIN_NAME
        expected.ipAddress = IP_ADDRESS
        expected.hostCpid = HOST_CPID
        expected.noOfCPUs = 1
        expected.cpuVendor = VENDOR
        expected.cpuModel = MODEL
        expected.cpuFeatures = FEATURES
        expected.cpuFloatingPointOps = 1.5
        expected.cpuIntegerOps = 1.5
        expected.cpuMembw = 1.5
        expected.cpuCalculated = 0L
        expected.productName = PRODUCT_NAME
        expected.memoryInBytes = BYTES_IN_1_GB.toDouble()
        expected.memoryCache = BYTES_IN_1_GB.toDouble()
        expected.memorySwap = BYTES_IN_1_GB.toDouble()
        expected.totalDiskSpace = TOTAL_SPACE.toDouble()
        expected.freeDiskSpace = FREE_SPACE.toDouble()
        expected.osName = OS_NAME
        expected.osVersion = OS_VERSION
        expected.virtualBoxVersion = VIRTUALBOX_VERSION
        Assert.assertEquals(expected, hostInfoParser.hostInfo)
    }

    companion object {
        private const val BYTES_IN_1_GB = 1073741824
        private const val BYTES_IN_1_GB_STR = BYTES_IN_1_GB.toString()
        private const val DOMAIN_NAME = "Domain Name"
        private const val IP_ADDRESS = "IP Address"
        private const val HOST_CPID = "Host CPID"
        private const val VENDOR = "Vendor"
        private const val MODEL = "Model"
        private const val FEATURES = "Features"
        private const val PRODUCT_NAME = "Product Name"
        private const val TOTAL_SPACE = BYTES_IN_1_GB * 16L
        private const val TOTAL_SPACE_STR = TOTAL_SPACE.toString()
        private const val FREE_SPACE = BYTES_IN_1_GB * 12L
        private const val FREE_SPACE_STR = FREE_SPACE.toString()
        private const val OS_NAME = "Android"
        private const val OS_VERSION = "Q"
        private const val VIRTUALBOX_VERSION = "6.0.18"
    }
}
