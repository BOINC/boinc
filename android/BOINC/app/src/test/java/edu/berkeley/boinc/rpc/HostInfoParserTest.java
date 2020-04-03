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
package edu.berkeley.boinc.rpc;

import android.util.Xml;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.xml.sax.SAXException;

import kotlin.UninitializedPropertyAccessException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class HostInfoParserTest {
    private static final int BYTES_IN_1_GB = 1073741824;
    private static final String BYTES_IN_1_GB_STR = Integer.toString(BYTES_IN_1_GB);

    private static final String DOMAIN_NAME = "Domain Name";
    private static final String IP_ADDRESS = "IP Address";
    private static final String HOST_CPID = "Host CPID";
    private static final String VENDOR = "Vendor";
    private static final String MODEL = "Model";
    private static final String FEATURES = "Features";
    private static final String PRODUCT_NAME = "Product Name";
    private static final long TOTAL_SPACE = BYTES_IN_1_GB * 16L;
    private static final String TOTAL_SPACE_STR = Long.toString(TOTAL_SPACE);
    private static final long FREE_SPACE = BYTES_IN_1_GB * 12L;
    private static final String FREE_SPACE_STR = Long.toString(FREE_SPACE);
    private static final String OS_NAME = "Android";
    private static final String OS_VERSION = "Q";
    private static final String VIRTUALBOX_VERSION = "6.0.18";

    private HostInfoParser hostInfoParser;
    private HostInfo expected;

    @Before
    public void setUp() {
        hostInfoParser = new HostInfoParser();
        expected = new HostInfo();
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParse_whenRpcStringIsNull_thenExpectUninitializedPropertyAccessException() {
        mockStatic(Xml.class);

        HostInfoParser.parse(null);
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParse_whenRpcStringIsEmpty_thenExpectUninitializedPropertyAccessException() {
        mockStatic(Xml.class);

        HostInfoParser.parse("");
    }

    @Test(expected = IllegalArgumentException.class)
    public void testParser_whenLocalNameIsNull_thenExpectIllegalArgumentException() throws SAXException {
        hostInfoParser.startElement(null, null, null, null);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectElementStarted() throws SAXException {
        hostInfoParser.startElement(null, "", null, null);

        assertTrue(hostInfoParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsHostInfoTag_thenExpectElementNotStarted() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        assertFalse(hostInfoParser.mElementStarted);
    }

    @Test(expected = UninitializedPropertyAccessException.class)
    public void testParser_whenLocalNameIsEmpty_thenExpectUninitializedPropertyAccessException()
            throws SAXException {
        hostInfoParser.startElement(null, "", null, null);

        hostInfoParser.getHostInfo();
    }

    @Test
    public void testParser_whenLocalNameIsHostInfoTag_thenExpectDefaultHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasTimezone_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.TIMEZONE, null, null);
        hostInfoParser.characters("1".toCharArray(), 0, 1);
        hostInfoParser.endElement(null, HostInfo.Fields.TIMEZONE, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setTimezone(1);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasDomainNameAndInvalidTimezone_thenExpectHostInfoWithOnlyDomainName()
            throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.DOMAIN_NAME, null, null);
        hostInfoParser.characters(DOMAIN_NAME.toCharArray(), 0, 11);
        hostInfoParser.endElement(null, HostInfo.Fields.DOMAIN_NAME, null);
        hostInfoParser.startElement(null, HostInfo.Fields.TIMEZONE, null, null);
        hostInfoParser.characters("One".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.TIMEZONE, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setDomainName(DOMAIN_NAME);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasDomainName_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.DOMAIN_NAME, null, null);
        hostInfoParser.characters(DOMAIN_NAME.toCharArray(), 0, 11);
        hostInfoParser.endElement(null, HostInfo.Fields.DOMAIN_NAME, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setDomainName(DOMAIN_NAME);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasIpAddress_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.IP_ADDR, null, null);
        hostInfoParser.characters(IP_ADDRESS.toCharArray(), 0, IP_ADDRESS.length());
        hostInfoParser.endElement(null, HostInfo.Fields.IP_ADDR, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setIpAddress(IP_ADDRESS);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasHostCpid_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.HOST_CPID, null, null);
        hostInfoParser.characters(HOST_CPID.toCharArray(), 0, HOST_CPID.length());
        hostInfoParser.endElement(null, HostInfo.Fields.HOST_CPID, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setHostCpid(HOST_CPID);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasNoOfCPUs_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_NCPUS, null, null);
        hostInfoParser.characters("1".toCharArray(), 0, 1);
        hostInfoParser.endElement(null, HostInfo.Fields.P_NCPUS, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setNoOfCPUs(1);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasVendor_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_VENDOR, null, null);
        hostInfoParser.characters(VENDOR.toCharArray(), 0, VENDOR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.P_VENDOR, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setCpuVendor(VENDOR);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasModel_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_MODEL, null, null);
        hostInfoParser.characters(MODEL.toCharArray(), 0, MODEL.length());
        hostInfoParser.endElement(null, HostInfo.Fields.P_MODEL, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setCpuModel(MODEL);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasFeatures_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_FEATURES, null, null);
        hostInfoParser.characters(FEATURES.toCharArray(), 0, FEATURES.length());
        hostInfoParser.endElement(null, HostInfo.Fields.P_FEATURES, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setCpuFeatures(FEATURES);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasFloatingPointOps_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_FPOPS, null, null);
        hostInfoParser.characters("1.5".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.P_FPOPS, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setCpuFloatingPointOps(1.5);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasIntegerOps_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_IOPS, null, null);
        hostInfoParser.characters("1.5".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.P_IOPS, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setCpuIntegerOps(1.5);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasMembw_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_MEMBW, null, null);
        hostInfoParser.characters("1.5".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.P_MEMBW, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setCpuMembw(1.5);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasCalculated_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_CALCULATED, null, null);
        hostInfoParser.characters("0".toCharArray(), 0, 1);
        hostInfoParser.endElement(null, HostInfo.Fields.P_CALCULATED, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setCpuCalculated(0L);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasProductName_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.PRODUCT_NAME, null, null);
        hostInfoParser.characters(PRODUCT_NAME.toCharArray(), 0, PRODUCT_NAME.length());
        hostInfoParser.endElement(null, HostInfo.Fields.PRODUCT_NAME, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setProductName(PRODUCT_NAME);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasMemInBytes_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.M_NBYTES, null, null);
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.M_NBYTES, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setMemoryInBytes(1_073_741_824.0);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasMemCache_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.M_CACHE, null, null);
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.M_CACHE, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setMemoryCache(1_073_741_824.0);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasMemSwap_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.M_SWAP, null, null);
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.M_SWAP, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setMemorySwap(1_073_741_824.0);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasDiskTotal_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.D_TOTAL, null, null);
        hostInfoParser.characters(TOTAL_SPACE_STR.toCharArray(), 0, TOTAL_SPACE_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.D_TOTAL, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setTotalDiskSpace(TOTAL_SPACE);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasDiskFree_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.D_FREE, null, null);
        hostInfoParser.characters(FREE_SPACE_STR.toCharArray(), 0, FREE_SPACE_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.D_FREE, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setFreeDiskSpace(FREE_SPACE);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasOsName_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.OS_NAME, null, null);
        hostInfoParser.characters(OS_NAME.toCharArray(), 0, OS_NAME.length());
        hostInfoParser.endElement(null, HostInfo.Fields.OS_NAME, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setOsName(OS_NAME);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasOsVersion_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.OS_VERSION, null, null);
        hostInfoParser.characters(OS_VERSION.toCharArray(), 0, OS_VERSION.length());
        hostInfoParser.endElement(null, HostInfo.Fields.OS_VERSION, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setOsVersion(OS_VERSION);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasVirtualBoxVersion_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.VIRTUALBOX_VERSION, null, null);
        hostInfoParser.characters(VIRTUALBOX_VERSION.toCharArray(), 0, VIRTUALBOX_VERSION.length());
        hostInfoParser.endElement(null, HostInfo.Fields.VIRTUALBOX_VERSION, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setVirtualBoxVersion(VIRTUALBOX_VERSION);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasAllAttributes_thenExpectMatchingHostInfo()
            throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.TIMEZONE, null, null);
        hostInfoParser.characters("1".toCharArray(), 0, 1);
        hostInfoParser.endElement(null, HostInfo.Fields.TIMEZONE, null);
        hostInfoParser.startElement(null, HostInfo.Fields.DOMAIN_NAME, null, null);
        hostInfoParser.characters(DOMAIN_NAME.toCharArray(), 0, 11);
        hostInfoParser.endElement(null, HostInfo.Fields.DOMAIN_NAME, null);
        hostInfoParser.startElement(null, HostInfo.Fields.IP_ADDR, null, null);
        hostInfoParser.characters(IP_ADDRESS.toCharArray(), 0, IP_ADDRESS.length());
        hostInfoParser.endElement(null, HostInfo.Fields.IP_ADDR, null);
        hostInfoParser.startElement(null, HostInfo.Fields.HOST_CPID, null, null);
        hostInfoParser.characters(HOST_CPID.toCharArray(), 0, HOST_CPID.length());
        hostInfoParser.endElement(null, HostInfo.Fields.HOST_CPID, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_NCPUS, null, null);
        hostInfoParser.characters("1".toCharArray(), 0, 1);
        hostInfoParser.endElement(null, HostInfo.Fields.P_NCPUS, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_VENDOR, null, null);
        hostInfoParser.characters(VENDOR.toCharArray(), 0, VENDOR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.P_VENDOR, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_MODEL, null, null);
        hostInfoParser.characters(MODEL.toCharArray(), 0, MODEL.length());
        hostInfoParser.endElement(null, HostInfo.Fields.P_MODEL, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_FEATURES, null, null);
        hostInfoParser.characters(FEATURES.toCharArray(), 0, FEATURES.length());
        hostInfoParser.endElement(null, HostInfo.Fields.P_FEATURES, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_FPOPS, null, null);
        hostInfoParser.characters("1.5".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.P_FPOPS, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_IOPS, null, null);
        hostInfoParser.characters("1.5".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.P_IOPS, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_MEMBW, null, null);
        hostInfoParser.characters("1.5".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.P_MEMBW, null);
        hostInfoParser.startElement(null, HostInfo.Fields.P_CALCULATED, null, null);
        hostInfoParser.characters("0".toCharArray(), 0, 1);
        hostInfoParser.endElement(null, HostInfo.Fields.P_CALCULATED, null);
        hostInfoParser.startElement(null, HostInfo.Fields.PRODUCT_NAME, null, null);
        hostInfoParser.characters(PRODUCT_NAME.toCharArray(), 0, PRODUCT_NAME.length());
        hostInfoParser.endElement(null, HostInfo.Fields.PRODUCT_NAME, null);
        hostInfoParser.startElement(null, HostInfo.Fields.M_NBYTES, null, null);
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.M_NBYTES, null);
        hostInfoParser.startElement(null, HostInfo.Fields.M_CACHE, null, null);
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.M_CACHE, null);
        hostInfoParser.startElement(null, HostInfo.Fields.M_SWAP, null, null);
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.M_SWAP, null);
        hostInfoParser.startElement(null, HostInfo.Fields.D_TOTAL, null, null);
        hostInfoParser.characters(TOTAL_SPACE_STR.toCharArray(), 0, TOTAL_SPACE_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.D_TOTAL, null);
        hostInfoParser.startElement(null, HostInfo.Fields.D_FREE, null, null);
        hostInfoParser.characters(FREE_SPACE_STR.toCharArray(), 0, FREE_SPACE_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.D_FREE, null);
        hostInfoParser.startElement(null, HostInfo.Fields.OS_NAME, null, null);
        hostInfoParser.characters(OS_NAME.toCharArray(), 0, OS_NAME.length());
        hostInfoParser.endElement(null, HostInfo.Fields.OS_NAME, null);
        hostInfoParser.startElement(null, HostInfo.Fields.OS_VERSION, null, null);
        hostInfoParser.characters(OS_VERSION.toCharArray(), 0, OS_VERSION.length());
        hostInfoParser.endElement(null, HostInfo.Fields.OS_VERSION, null);
        hostInfoParser.startElement(null, HostInfo.Fields.VIRTUALBOX_VERSION, null, null);
        hostInfoParser.characters(VIRTUALBOX_VERSION.toCharArray(), 0, VIRTUALBOX_VERSION.length());
        hostInfoParser.endElement(null, HostInfo.Fields.VIRTUALBOX_VERSION, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        expected.setTimezone(1);
        expected.setDomainName(DOMAIN_NAME);
        expected.setIpAddress(IP_ADDRESS);
        expected.setHostCpid(HOST_CPID);
        expected.setNoOfCPUs(1);
        expected.setCpuVendor(VENDOR);
        expected.setCpuModel(MODEL);
        expected.setCpuFeatures(FEATURES);
        expected.setCpuFloatingPointOps(1.5);
        expected.setCpuIntegerOps(1.5);
        expected.setCpuMembw(1.5);
        expected.setCpuCalculated(0L);
        expected.setProductName(PRODUCT_NAME);
        expected.setMemoryInBytes(BYTES_IN_1_GB);
        expected.setMemoryCache(BYTES_IN_1_GB);
        expected.setMemorySwap(BYTES_IN_1_GB);
        expected.setTotalDiskSpace(TOTAL_SPACE);
        expected.setFreeDiskSpace(FREE_SPACE);
        expected.setOsName(OS_NAME);
        expected.setOsVersion(OS_VERSION);
        expected.setVirtualBoxVersion(VIRTUALBOX_VERSION);

        assertEquals(expected, hostInfoParser.getHostInfo());
    }
}
