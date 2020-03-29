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
import static org.junit.Assert.assertTrue;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class CcStateParserTest {
    /* Project test values */
    private static final int BYTES_IN_1_GB = 1073741824;
    private static final String BYTES_IN_1_GB_STR = Integer.toString(BYTES_IN_1_GB);

    private static final long TOTAL_SPACE = BYTES_IN_1_GB * 16L;
    private static final String TOTAL_SPACE_STR = Long.toString(TOTAL_SPACE);
    private static final long FREE_SPACE = BYTES_IN_1_GB * 12L;
    private static final String FREE_SPACE_STR = Long.toString(FREE_SPACE);

    private CcStateParser ccStateParser;
    private CcState expected;

    @Before
    public void setUp() {
        ccStateParser = new CcStateParser();
        expected = new CcState();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectDefaultCcState() {
        mockStatic(Xml.class);

        assertEquals(expected, CcStateParser.parse(null));
    }

    @Test
    public void testParse_whenRpcStringIsEmpty_thenExpectDefaultCcState() {
        mockStatic(Xml.class);

        assertEquals(expected, CcStateParser.parse(""));
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectElementNotStarted() throws SAXException {
        ccStateParser.startElement(null, "", null, null);

        assertFalse(ccStateParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsClientStateTag_thenExpectElementNotStarted()
            throws SAXException {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        assertFalse(ccStateParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectDefaultCcState() throws SAXException {
        ccStateParser.startElement(null, "", null, null);

        assertEquals(expected, ccStateParser.getCcState());
    }

    @Test
    public void testParser_whenLocalNameIsClientStateTag_thenExpectDefaultCcState() throws SAXException {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        assertEquals(expected, ccStateParser.getCcState());
    }

    @Test
    public void testParser_whenEnteringXmlCcState_thenExpectEmptyCcStateLists() throws SAXException {
        final CcState ccState = ccStateParser.getCcState();

        ccState.getApps().add(new App());
        ccState.getAppVersions().add(new AppVersion());
        ccState.getProjects().add(new Project());
        ccState.getResults().add(new Result());
        ccState.getWorkUnits().add(new WorkUnit());

        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        assertTrue(ccState.getApps().isEmpty());
        assertTrue(ccState.getAppVersions().isEmpty());
        assertTrue(ccState.getProjects().isEmpty());
        assertTrue(ccState.getResults().isEmpty());
        assertTrue(ccState.getWorkUnits().isEmpty());
    }

    @Test
    public void testParser_whenXmlCcStateHasApp_thenExpectMatchingCcState()
            throws SAXException {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null);
        ccStateParser.startElement(null, AppsParser.APP_TAG, null, null);
        ccStateParser.startElement(null, RPCCommonTags.NAME, null, null);
        ccStateParser.characters("App".toCharArray(), 0, 3);
        ccStateParser.endElement(null, RPCCommonTags.NAME, null);
        ccStateParser.startElement(null, RPCCommonTags.USER_FRIENDLY_NAME, null, null);
        ccStateParser.characters("App (Friendly Name)".toCharArray(), 0, "App (Friendly Name)".length());
        ccStateParser.endElement(null, RPCCommonTags.USER_FRIENDLY_NAME, null);
        ccStateParser.startElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, RPCCommonTags.NON_CPU_INTENSIVE, null);
        ccStateParser.endElement(null, AppsParser.APP_TAG, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        final App expectedApp = new App("App", "App (Friendly Name)", 1);
        expectedApp.setProject(new Project());

        expected.getApps().add(expectedApp);

        assertEquals(expected, ccStateParser.getCcState());
    }

    @Test
    public void testParser_whenXmlCcStateHasAppVersion_thenExpectMatchingCcState()
            throws SAXException {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null);
        ccStateParser.startElement(null, AppVersionsParser.APP_VERSION_TAG, null, null);
        ccStateParser.startElement(null, AppVersion.Fields.APP_NAME, null, null);
        ccStateParser.characters("App".toCharArray(), 0, 3);
        ccStateParser.endElement(null, AppVersion.Fields.APP_NAME, null);
        ccStateParser.startElement(null, AppVersion.Fields.VERSION_NUM, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, AppVersion.Fields.VERSION_NUM, null);
        ccStateParser.endElement(null, AppVersionsParser.APP_VERSION_TAG, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        final AppVersion expectedAppVersion = new AppVersion("App", 1);
        expectedAppVersion.setProject(new Project());

        expected.getAppVersions().add(expectedAppVersion);

        assertEquals(expected, ccStateParser.getCcState());
    }

    @Test
    public void testParser_whenXmlCcStateHasHostInfo_thenExpectMatchingCcState()
            throws SAXException {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null);
        ccStateParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null);
        ccStateParser.startElement(null, HostInfo.Fields.TIMEZONE, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, HostInfo.Fields.TIMEZONE, null);
        ccStateParser.startElement(null, HostInfo.Fields.DOMAIN_NAME, null, null);
        ccStateParser.characters("Domain Name".toCharArray(), 0, "Domain Name".length());
        ccStateParser.endElement(null, HostInfo.Fields.DOMAIN_NAME, null);
        ccStateParser.startElement(null, HostInfo.Fields.IP_ADDR, null, null);
        ccStateParser.characters("IP Address".toCharArray(), 0, "IP Address".length());
        ccStateParser.endElement(null, HostInfo.Fields.IP_ADDR, null);
        ccStateParser.startElement(null, HostInfo.Fields.HOST_CPID, null, null);
        ccStateParser.characters("Host CPID".toCharArray(), 0, "Host CPID".length());
        ccStateParser.endElement(null, HostInfo.Fields.HOST_CPID, null);
        ccStateParser.startElement(null, HostInfo.Fields.P_NCPUS, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, HostInfo.Fields.P_NCPUS, null);
        ccStateParser.startElement(null, HostInfo.Fields.P_VENDOR, null, null);
        ccStateParser.characters("Vendor".toCharArray(), 0, "Vendor".length());
        ccStateParser.endElement(null, HostInfo.Fields.P_VENDOR, null);
        ccStateParser.startElement(null, HostInfo.Fields.P_MODEL, null, null);
        ccStateParser.characters("Model".toCharArray(), 0, "Model".length());
        ccStateParser.endElement(null, HostInfo.Fields.P_MODEL, null);
        ccStateParser.startElement(null, HostInfo.Fields.P_FEATURES, null, null);
        ccStateParser.characters("Features".toCharArray(), 0, "Features".length());
        ccStateParser.endElement(null, HostInfo.Fields.P_FEATURES, null);
        ccStateParser.startElement(null, HostInfo.Fields.P_FPOPS, null, null);
        ccStateParser.characters("1.5".toCharArray(), 0, 3);
        ccStateParser.endElement(null, HostInfo.Fields.P_FPOPS, null);
        ccStateParser.startElement(null, HostInfo.Fields.P_IOPS, null, null);
        ccStateParser.characters("1.5".toCharArray(), 0, 3);
        ccStateParser.endElement(null, HostInfo.Fields.P_IOPS, null);
        ccStateParser.startElement(null, HostInfo.Fields.P_MEMBW, null, null);
        ccStateParser.characters("1.5".toCharArray(), 0, 3);
        ccStateParser.endElement(null, HostInfo.Fields.P_MEMBW, null);
        ccStateParser.startElement(null, HostInfo.Fields.P_CALCULATED, null, null);
        ccStateParser.characters("0".toCharArray(), 0, 1);
        ccStateParser.endElement(null, HostInfo.Fields.P_CALCULATED, null);
        ccStateParser.startElement(null, HostInfo.Fields.PRODUCT_NAME, null, null);
        ccStateParser.characters("Product Name".toCharArray(), 0, "Product Name".length());
        ccStateParser.endElement(null, HostInfo.Fields.PRODUCT_NAME, null);
        ccStateParser.startElement(null, HostInfo.Fields.M_NBYTES, null, null);
        ccStateParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        ccStateParser.endElement(null, HostInfo.Fields.M_NBYTES, null);
        ccStateParser.startElement(null, HostInfo.Fields.M_CACHE, null, null);
        ccStateParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        ccStateParser.endElement(null, HostInfo.Fields.M_CACHE, null);
        ccStateParser.startElement(null, HostInfo.Fields.M_SWAP, null, null);
        ccStateParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        ccStateParser.endElement(null, HostInfo.Fields.M_SWAP, null);
        ccStateParser.startElement(null, HostInfo.Fields.D_TOTAL, null, null);
        ccStateParser.characters(TOTAL_SPACE_STR.toCharArray(), 0, TOTAL_SPACE_STR.length());
        ccStateParser.endElement(null, HostInfo.Fields.D_TOTAL, null);
        ccStateParser.startElement(null, HostInfo.Fields.D_FREE, null, null);
        ccStateParser.characters(FREE_SPACE_STR.toCharArray(), 0, FREE_SPACE_STR.length());
        ccStateParser.endElement(null, HostInfo.Fields.D_FREE, null);
        ccStateParser.startElement(null, HostInfo.Fields.OS_NAME, null, null);
        ccStateParser.characters("Android".toCharArray(), 0, "Android".length());
        ccStateParser.endElement(null, HostInfo.Fields.OS_NAME, null);
        ccStateParser.startElement(null, HostInfo.Fields.OS_VERSION, null, null);
        ccStateParser.characters("Q".toCharArray(), 0, "Q".length());
        ccStateParser.endElement(null, HostInfo.Fields.OS_VERSION, null);
        ccStateParser.startElement(null, HostInfo.Fields.VIRTUALBOX_VERSION, null, null);
        ccStateParser.characters("6.0.18".toCharArray(), 0, "6.0.18".length());
        ccStateParser.endElement(null, HostInfo.Fields.VIRTUALBOX_VERSION, null);
        ccStateParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        final HostInfo expectedHostInfo = new HostInfo();
        expectedHostInfo.setTimezone(1);
        expectedHostInfo.setDomainName("Domain Name");
        expectedHostInfo.setIpAddress("IP Address");
        expectedHostInfo.setHostCpid("Host CPID");
        expectedHostInfo.setNoOfCPUs(1);
        expectedHostInfo.setCpuVendor("Vendor");
        expectedHostInfo.setCpuModel("Model");
        expectedHostInfo.setCpuFeatures("Features");
        expectedHostInfo.setCpuFloatingPointOps(1.5);
        expectedHostInfo.setCpuIntegerOps(1.5);
        expectedHostInfo.setCpuMembw(1.5);
        expectedHostInfo.setCpuCalculated(0L);
        expectedHostInfo.setProductName("Product Name");
        expectedHostInfo.setMemoryInBytes(BYTES_IN_1_GB);
        expectedHostInfo.setMemoryCache(BYTES_IN_1_GB);
        expectedHostInfo.setMemorySwap(BYTES_IN_1_GB);
        expectedHostInfo.setTotalDiskSpace(TOTAL_SPACE);
        expectedHostInfo.setFreeDiskSpace(FREE_SPACE);
        expectedHostInfo.setOsName("Android");
        expectedHostInfo.setOsVersion("Q");
        expectedHostInfo.setVirtualBoxVersion("6.0.18");

        expected.setHostInfo(expectedHostInfo);

        assertEquals(expected, ccStateParser.getCcState());
    }

    @Test
    public void testParser_whenXmlCcStateHasProject_thenExpectMatchingCcState()
            throws SAXException {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null);
        ccStateParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        ccStateParser.startElement(null, RPCCommonTags.MASTER_URL, null, null);
        ccStateParser.characters("Master URL".toCharArray(), 0, "Master URL".length());
        ccStateParser.endElement(null, RPCCommonTags.MASTER_URL, null);
        ccStateParser.startElement(null, RPCCommonTags.GUI_URL, null, null);
        ccStateParser.startElement(null, RPCCommonTags.NAME, null, null);
        ccStateParser.characters("GUI URL Name".toCharArray(), 0, "GUI URL Name".length());
        ccStateParser.endElement(null, RPCCommonTags.NAME, null);
        ccStateParser.startElement(null, RPCCommonTags.DESCRIPTION, null, null);
        ccStateParser.characters("GUI URL Description".toCharArray(), 0, "GUI URL Description".length());
        ccStateParser.endElement(null, RPCCommonTags.DESCRIPTION, null);
        ccStateParser.startElement(null, RPCCommonTags.URL, null, null);
        ccStateParser.characters("GUI URL URL".toCharArray(), 0, "GUI URL URL".length());
        ccStateParser.endElement(null, RPCCommonTags.URL, null);
        ccStateParser.endElement(null, RPCCommonTags.GUI_URL, null);
        ccStateParser.startElement(null, Project.Fields.NO_ATI_PREF, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, Project.Fields.NO_ATI_PREF, null);
        ccStateParser.endElement(null, RPCCommonTags.PROJECT, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        final Project expectedProject = new Project();
        expectedProject.setMasterURL("Master URL");
        expectedProject.getGuiURLs().add(new GuiUrl("GUI URL Name", "GUI URL Description",
                                               "GUI URL URL"));
        expectedProject.setNoATIPref(true);

        expected.getProjects().add(expectedProject);

        assertEquals(expected, ccStateParser.getCcState());
    }

    @Test
    public void testParser_whenXmlCcStateHasResult_thenExpectMatchingCcState()
            throws SAXException {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null);
        ccStateParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        ccStateParser.startElement(null, RPCCommonTags.NAME, null, null);
        ccStateParser.characters("Result".toCharArray(), 0, "Result".length());
        ccStateParser.endElement(null, RPCCommonTags.NAME, null);
        ccStateParser.startElement(null, Result.Fields.active_task, null, null);
        ccStateParser.startElement(null, Result.Fields.slot_path, null, null);
        ccStateParser.characters("/path/to/slot".toCharArray(), 0, "/path/to/slot".length());
        ccStateParser.endElement(null, Result.Fields.slot_path, null);
        ccStateParser.endElement(null, Result.Fields.active_task, null);
        ccStateParser.endElement(null, ResultsParser.RESULT_TAG, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        final Result expectedResult = new Result();
        expectedResult.active_task = true;
        expectedResult.name = "Result";
        expectedResult.project = new Project();
        expectedResult.slot_path = "/path/to/slot";

        expected.getResults().add(expectedResult);

        assertEquals(expected, ccStateParser.getCcState());
    }

    @Test
    public void testParser_whenXmlCcStateHasVersionInfoWithInvalidMajorVersion_thenExpectCcStateWithDefaultMajorVersion()
            throws SAXException {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null);
        ccStateParser.startElement(null, CcStateParser.CORE_CLIENT_MAJOR_VERSION_TAG, null, null);
        ccStateParser.characters("One".toCharArray(), 0, 3);
        ccStateParser.endElement(null, CcStateParser.CORE_CLIENT_MAJOR_VERSION_TAG, null);
        ccStateParser.startElement(null, CcStateParser.CORE_CLIENT_MINOR_VERSION_TAG, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, CcStateParser.CORE_CLIENT_MINOR_VERSION_TAG, null);
        ccStateParser.startElement(null, CcStateParser.CORE_CLIENT_RELEASE_TAG, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, CcStateParser.CORE_CLIENT_RELEASE_TAG, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        expected.setVersionInfo(new VersionInfo(0, 1, 1));

        assertEquals(expected, ccStateParser.getCcState());
    }

    @Test
    public void testParser_whenXmlCcStateHasVersionInfo_thenExpectMatchingCcState()
            throws SAXException {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null);
        ccStateParser.startElement(null, CcStateParser.CORE_CLIENT_MAJOR_VERSION_TAG, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, CcStateParser.CORE_CLIENT_MAJOR_VERSION_TAG, null);
        ccStateParser.startElement(null, CcStateParser.CORE_CLIENT_MINOR_VERSION_TAG, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, CcStateParser.CORE_CLIENT_MINOR_VERSION_TAG, null);
        ccStateParser.startElement(null, CcStateParser.CORE_CLIENT_RELEASE_TAG, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, CcStateParser.CORE_CLIENT_RELEASE_TAG, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        expected.setVersionInfo(new VersionInfo(1, 1, 1));

        assertEquals(expected, ccStateParser.getCcState());
    }

    @Test
    public void testParser_whenXmlCcStateHasWorkUnit_thenExpectMatchingCcState() throws SAXException {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null);
        ccStateParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null);
        ccStateParser.startElement(null, RPCCommonTags.NAME, null, null);
        ccStateParser.characters("Work unit".toCharArray(), 0, "Work unit".length());
        ccStateParser.endElement(null, RPCCommonTags.NAME, null);
        ccStateParser.startElement(null, WorkUnit.Fields.RSC_DISK_BOUND, null, null);
        ccStateParser.characters("1.5".toCharArray(), 0, 3);
        ccStateParser.endElement(null, WorkUnit.Fields.RSC_DISK_BOUND, null);
        ccStateParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        final WorkUnit expectedWorkUnit = new WorkUnit();
        expectedWorkUnit.setName("Work unit");
        expectedWorkUnit.setRscDiskBound(1.5);
        expectedWorkUnit.setProject(new Project());

        expected.getWorkUnits().add(expectedWorkUnit);

        assertEquals(expected, ccStateParser.getCcState());
    }
}
