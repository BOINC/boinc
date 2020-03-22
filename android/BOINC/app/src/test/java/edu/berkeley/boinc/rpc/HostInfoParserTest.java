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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertThrows;
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

    @Before
    public void setUp() {
        hostInfoParser = new HostInfoParser();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectNull() {
        mockStatic(Xml.class);

        assertNull(HostInfoParser.parse(null));
    }

    @Test
    public void testParse_whenRpcStringIsEmpty_thenExpectNull() {
        mockStatic(Xml.class);

        assertNull(HostInfoParser.parse(""));
    }

    @Test
    public void testParser_whenLocalNameIsNull_thenExpectNullPointerExceptionAndNullHostInfo() {
        assertThrows(NullPointerException.class, () -> hostInfoParser.startElement(null, null, null, null));
        assertNull(hostInfoParser.getHostInfo());
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

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectNullHostInfo() throws SAXException {
        hostInfoParser.startElement(null, "", null, null);

        assertNull(hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenLocalNameIsHostInfoTag_thenExpectDefaultHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        assertEquals(new HostInfo(), hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasTimezone_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.timezone, null, null);
        hostInfoParser.characters("1".toCharArray(), 0, 1);
        hostInfoParser.endElement(null, HostInfo.Fields.timezone, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.timezone = 1;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasDomainNameAndInvalidTimezone_thenExpectHostInfoWithOnlyDomainName()
            throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.domain_name, null, null);
        hostInfoParser.characters(DOMAIN_NAME.toCharArray(), 0, 11);
        hostInfoParser.endElement(null, HostInfo.Fields.domain_name, null);
        hostInfoParser.startElement(null, HostInfo.Fields.timezone, null, null);
        hostInfoParser.characters("One".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.timezone, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.domain_name = DOMAIN_NAME;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasDomainName_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.domain_name, null, null);
        hostInfoParser.characters(DOMAIN_NAME.toCharArray(), 0, 11);
        hostInfoParser.endElement(null, HostInfo.Fields.domain_name, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.domain_name = DOMAIN_NAME;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasIpAddress_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.ip_addr, null, null);
        hostInfoParser.characters(IP_ADDRESS.toCharArray(), 0, IP_ADDRESS.length());
        hostInfoParser.endElement(null, HostInfo.Fields.ip_addr, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.ip_addr = IP_ADDRESS;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasHostCpid_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.host_cpid, null, null);
        hostInfoParser.characters(HOST_CPID.toCharArray(), 0, HOST_CPID.length());
        hostInfoParser.endElement(null, HostInfo.Fields.host_cpid, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.host_cpid = HOST_CPID;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasNoOfCPUs_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_ncpus, null, null);
        hostInfoParser.characters("1".toCharArray(), 0, 1);
        hostInfoParser.endElement(null, HostInfo.Fields.p_ncpus, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.p_ncpus = 1;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasVendor_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_vendor, null, null);
        hostInfoParser.characters(VENDOR.toCharArray(), 0, VENDOR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.p_vendor, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.p_vendor = VENDOR;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasModel_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_model, null, null);
        hostInfoParser.characters(MODEL.toCharArray(), 0, MODEL.length());
        hostInfoParser.endElement(null, HostInfo.Fields.p_model, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.p_model = MODEL;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasFeatures_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_features, null, null);
        hostInfoParser.characters(FEATURES.toCharArray(), 0, FEATURES.length());
        hostInfoParser.endElement(null, HostInfo.Fields.p_features, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.p_features = FEATURES;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasFloatingPointOps_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_fpops, null, null);
        hostInfoParser.characters("1.5".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.p_fpops, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.p_fpops = 1.5;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasIntegerOps_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_iops, null, null);
        hostInfoParser.characters("1.5".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.p_iops, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.p_iops = 1.5;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasMembw_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_membw, null, null);
        hostInfoParser.characters("1.5".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.p_membw, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.p_membw = 1.5;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasCalculated_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_calculated, null, null);
        hostInfoParser.characters("0".toCharArray(), 0, 1);
        hostInfoParser.endElement(null, HostInfo.Fields.p_calculated, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.p_calculated = 0L;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasProductName_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.product_name, null, null);
        hostInfoParser.characters(PRODUCT_NAME.toCharArray(), 0, PRODUCT_NAME.length());
        hostInfoParser.endElement(null, HostInfo.Fields.product_name, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.product_name = PRODUCT_NAME;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasMemInBytes_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.m_nbytes, null, null);
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.m_nbytes, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.m_nbytes = 1_073_741_824.0;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasMemCache_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.m_cache, null, null);
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.m_cache, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.m_cache = 1_073_741_824.0;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasMemSwap_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.m_swap, null, null);
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.m_swap, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.m_swap = 1_073_741_824.0;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasDiskTotal_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.d_total, null, null);
        hostInfoParser.characters(TOTAL_SPACE_STR.toCharArray(), 0, TOTAL_SPACE_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.d_total, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.d_total = TOTAL_SPACE;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasDiskFree_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.d_free, null, null);
        hostInfoParser.characters(FREE_SPACE_STR.toCharArray(), 0, FREE_SPACE_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.d_free, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.d_free = FREE_SPACE;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasOsName_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.os_name, null, null);
        hostInfoParser.characters(OS_NAME.toCharArray(), 0, OS_NAME.length());
        hostInfoParser.endElement(null, HostInfo.Fields.os_name, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.os_name = OS_NAME;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasOsVersion_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.os_version, null, null);
        hostInfoParser.characters(OS_VERSION.toCharArray(), 0, OS_VERSION.length());
        hostInfoParser.endElement(null, HostInfo.Fields.os_version, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.os_version = OS_VERSION;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasVirtualBoxVersion_thenExpectMatchingHostInfo() throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.virtualbox_version, null, null);
        hostInfoParser.characters(VIRTUALBOX_VERSION.toCharArray(), 0, VIRTUALBOX_VERSION.length());
        hostInfoParser.endElement(null, HostInfo.Fields.virtualbox_version, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.virtualbox_version = VIRTUALBOX_VERSION;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }

    @Test
    public void testParser_whenXmlHostInfoHasAllAttributes_thenExpectMatchingHostInfo()
            throws SAXException {
        hostInfoParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        hostInfoParser.startElement(null, HostInfo.Fields.timezone, null, null);
        hostInfoParser.characters("1".toCharArray(), 0, 1);
        hostInfoParser.endElement(null, HostInfo.Fields.timezone, null);
        hostInfoParser.startElement(null, HostInfo.Fields.domain_name, null, null);
        hostInfoParser.characters(DOMAIN_NAME.toCharArray(), 0, 11);
        hostInfoParser.endElement(null, HostInfo.Fields.domain_name, null);
        hostInfoParser.startElement(null, HostInfo.Fields.ip_addr, null, null);
        hostInfoParser.characters(IP_ADDRESS.toCharArray(), 0, IP_ADDRESS.length());
        hostInfoParser.endElement(null, HostInfo.Fields.ip_addr, null);
        hostInfoParser.startElement(null, HostInfo.Fields.host_cpid, null, null);
        hostInfoParser.characters(HOST_CPID.toCharArray(), 0, HOST_CPID.length());
        hostInfoParser.endElement(null, HostInfo.Fields.host_cpid, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_ncpus, null, null);
        hostInfoParser.characters("1".toCharArray(), 0, 1);
        hostInfoParser.endElement(null, HostInfo.Fields.p_ncpus, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_vendor, null, null);
        hostInfoParser.characters(VENDOR.toCharArray(), 0, VENDOR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.p_vendor, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_model, null, null);
        hostInfoParser.characters(MODEL.toCharArray(), 0, MODEL.length());
        hostInfoParser.endElement(null, HostInfo.Fields.p_model, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_features, null, null);
        hostInfoParser.characters(FEATURES.toCharArray(), 0, FEATURES.length());
        hostInfoParser.endElement(null, HostInfo.Fields.p_features, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_fpops, null, null);
        hostInfoParser.characters("1.5".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.p_fpops, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_iops, null, null);
        hostInfoParser.characters("1.5".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.p_iops, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_membw, null, null);
        hostInfoParser.characters("1.5".toCharArray(), 0, 3);
        hostInfoParser.endElement(null, HostInfo.Fields.p_membw, null);
        hostInfoParser.startElement(null, HostInfo.Fields.p_calculated, null, null);
        hostInfoParser.characters("0".toCharArray(), 0, 1);
        hostInfoParser.endElement(null, HostInfo.Fields.p_calculated, null);
        hostInfoParser.startElement(null, HostInfo.Fields.product_name, null, null);
        hostInfoParser.characters(PRODUCT_NAME.toCharArray(), 0, PRODUCT_NAME.length());
        hostInfoParser.endElement(null, HostInfo.Fields.product_name, null);
        hostInfoParser.startElement(null, HostInfo.Fields.m_nbytes, null, null);
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.m_nbytes, null);
        hostInfoParser.startElement(null, HostInfo.Fields.m_cache, null, null);
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.m_cache, null);
        hostInfoParser.startElement(null, HostInfo.Fields.m_swap, null, null);
        hostInfoParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.m_swap, null);
        hostInfoParser.startElement(null, HostInfo.Fields.d_total, null, null);
        hostInfoParser.characters(TOTAL_SPACE_STR.toCharArray(), 0, TOTAL_SPACE_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.d_total, null);
        hostInfoParser.startElement(null, HostInfo.Fields.d_free, null, null);
        hostInfoParser.characters(FREE_SPACE_STR.toCharArray(), 0, FREE_SPACE_STR.length());
        hostInfoParser.endElement(null, HostInfo.Fields.d_free, null);
        hostInfoParser.startElement(null, HostInfo.Fields.os_name, null, null);
        hostInfoParser.characters(OS_NAME.toCharArray(), 0, OS_NAME.length());
        hostInfoParser.endElement(null, HostInfo.Fields.os_name, null);
        hostInfoParser.startElement(null, HostInfo.Fields.os_version, null, null);
        hostInfoParser.characters(OS_VERSION.toCharArray(), 0, OS_VERSION.length());
        hostInfoParser.endElement(null, HostInfo.Fields.os_version, null);
        hostInfoParser.startElement(null, HostInfo.Fields.virtualbox_version, null, null);
        hostInfoParser.characters(VIRTUALBOX_VERSION.toCharArray(), 0, VIRTUALBOX_VERSION.length());
        hostInfoParser.endElement(null, HostInfo.Fields.virtualbox_version, null);
        hostInfoParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);

        final HostInfo expected = new HostInfo();
        expected.timezone = 1;
        expected.domain_name = DOMAIN_NAME;
        expected.ip_addr = IP_ADDRESS;
        expected.host_cpid = HOST_CPID;
        expected.p_ncpus = 1;
        expected.p_vendor = VENDOR;
        expected.p_model = MODEL;
        expected.p_features = FEATURES;
        expected.p_fpops = 1.5;
        expected.p_iops = 1.5;
        expected.p_membw = 1.5;
        expected.p_calculated = 0L;
        expected.product_name = PRODUCT_NAME;
        expected.m_nbytes = 1_073_741_824.0;
        expected.m_cache = 1_073_741_824.0;
        expected.m_swap = 1_073_741_824.0;
        expected.d_total = TOTAL_SPACE;
        expected.d_free = FREE_SPACE;
        expected.os_name = OS_NAME;
        expected.os_version = OS_VERSION;
        expected.virtualbox_version = VIRTUALBOX_VERSION;

        assertEquals(expected, hostInfoParser.getHostInfo());
    }
}