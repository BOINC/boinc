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
        ccState.getWorkunits().add(new Workunit());

        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        assertTrue(ccState.getApps().isEmpty());
        assertTrue(ccState.getAppVersions().isEmpty());
        assertTrue(ccState.getProjects().isEmpty());
        assertTrue(ccState.getResults().isEmpty());
        assertTrue(ccState.getWorkunits().isEmpty());
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
        ccStateParser.startElement(null, HostInfo.Fields.timezone, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, HostInfo.Fields.timezone, null);
        ccStateParser.startElement(null, HostInfo.Fields.domain_name, null, null);
        ccStateParser.characters("Domain Name".toCharArray(), 0, "Domain Name".length());
        ccStateParser.endElement(null, HostInfo.Fields.domain_name, null);
        ccStateParser.startElement(null, HostInfo.Fields.ip_addr, null, null);
        ccStateParser.characters("IP Address".toCharArray(), 0, "IP Address".length());
        ccStateParser.endElement(null, HostInfo.Fields.ip_addr, null);
        ccStateParser.startElement(null, HostInfo.Fields.host_cpid, null, null);
        ccStateParser.characters("Host CPID".toCharArray(), 0, "Host CPID".length());
        ccStateParser.endElement(null, HostInfo.Fields.host_cpid, null);
        ccStateParser.startElement(null, HostInfo.Fields.p_ncpus, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, HostInfo.Fields.p_ncpus, null);
        ccStateParser.startElement(null, HostInfo.Fields.p_vendor, null, null);
        ccStateParser.characters("Vendor".toCharArray(), 0, "Vendor".length());
        ccStateParser.endElement(null, HostInfo.Fields.p_vendor, null);
        ccStateParser.startElement(null, HostInfo.Fields.p_model, null, null);
        ccStateParser.characters("Model".toCharArray(), 0, "Model".length());
        ccStateParser.endElement(null, HostInfo.Fields.p_model, null);
        ccStateParser.startElement(null, HostInfo.Fields.p_features, null, null);
        ccStateParser.characters("Features".toCharArray(), 0, "Features".length());
        ccStateParser.endElement(null, HostInfo.Fields.p_features, null);
        ccStateParser.startElement(null, HostInfo.Fields.p_fpops, null, null);
        ccStateParser.characters("1.5".toCharArray(), 0, 3);
        ccStateParser.endElement(null, HostInfo.Fields.p_fpops, null);
        ccStateParser.startElement(null, HostInfo.Fields.p_iops, null, null);
        ccStateParser.characters("1.5".toCharArray(), 0, 3);
        ccStateParser.endElement(null, HostInfo.Fields.p_iops, null);
        ccStateParser.startElement(null, HostInfo.Fields.p_membw, null, null);
        ccStateParser.characters("1.5".toCharArray(), 0, 3);
        ccStateParser.endElement(null, HostInfo.Fields.p_membw, null);
        ccStateParser.startElement(null, HostInfo.Fields.p_calculated, null, null);
        ccStateParser.characters("0".toCharArray(), 0, 1);
        ccStateParser.endElement(null, HostInfo.Fields.p_calculated, null);
        ccStateParser.startElement(null, HostInfo.Fields.product_name, null, null);
        ccStateParser.characters("Product Name".toCharArray(), 0, "Product Name".length());
        ccStateParser.endElement(null, HostInfo.Fields.product_name, null);
        ccStateParser.startElement(null, HostInfo.Fields.m_nbytes, null, null);
        ccStateParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        ccStateParser.endElement(null, HostInfo.Fields.m_nbytes, null);
        ccStateParser.startElement(null, HostInfo.Fields.m_cache, null, null);
        ccStateParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        ccStateParser.endElement(null, HostInfo.Fields.m_cache, null);
        ccStateParser.startElement(null, HostInfo.Fields.m_swap, null, null);
        ccStateParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length());
        ccStateParser.endElement(null, HostInfo.Fields.m_swap, null);
        ccStateParser.startElement(null, HostInfo.Fields.d_total, null, null);
        ccStateParser.characters(TOTAL_SPACE_STR.toCharArray(), 0, TOTAL_SPACE_STR.length());
        ccStateParser.endElement(null, HostInfo.Fields.d_total, null);
        ccStateParser.startElement(null, HostInfo.Fields.d_free, null, null);
        ccStateParser.characters(FREE_SPACE_STR.toCharArray(), 0, FREE_SPACE_STR.length());
        ccStateParser.endElement(null, HostInfo.Fields.d_free, null);
        ccStateParser.startElement(null, HostInfo.Fields.os_name, null, null);
        ccStateParser.characters("Android".toCharArray(), 0, "Android".length());
        ccStateParser.endElement(null, HostInfo.Fields.os_name, null);
        ccStateParser.startElement(null, HostInfo.Fields.os_version, null, null);
        ccStateParser.characters("Q".toCharArray(), 0, "Q".length());
        ccStateParser.endElement(null, HostInfo.Fields.os_version, null);
        ccStateParser.startElement(null, HostInfo.Fields.virtualbox_version, null, null);
        ccStateParser.characters("6.0.18".toCharArray(), 0, "6.0.18".length());
        ccStateParser.endElement(null, HostInfo.Fields.virtualbox_version, null);
        ccStateParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        final HostInfo expectedHostInfo = new HostInfo();
        expectedHostInfo.timezone = 1;
        expectedHostInfo.domain_name = "Domain Name";
        expectedHostInfo.ip_addr = "IP Address";
        expectedHostInfo.host_cpid = "Host CPID";
        expectedHostInfo.p_ncpus = 1;
        expectedHostInfo.p_vendor = "Vendor";
        expectedHostInfo.p_model = "Model";
        expectedHostInfo.p_features = "Features";
        expectedHostInfo.p_fpops = 1.5;
        expectedHostInfo.p_iops = 1.5;
        expectedHostInfo.p_membw = 1.5;
        expectedHostInfo.p_calculated = 0L;
        expectedHostInfo.product_name = "Product Name";
        expectedHostInfo.m_nbytes = BYTES_IN_1_GB;
        expectedHostInfo.m_cache = BYTES_IN_1_GB;
        expectedHostInfo.m_swap = BYTES_IN_1_GB;
        expectedHostInfo.d_total = TOTAL_SPACE;
        expectedHostInfo.d_free = FREE_SPACE;
        expectedHostInfo.os_name = "Android";
        expectedHostInfo.os_version = "Q";
        expectedHostInfo.virtualbox_version = "6.0.18";

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
        ccStateParser.startElement(null, Project.Fields.no_ati_pref, null, null);
        ccStateParser.characters("1".toCharArray(), 0, 1);
        ccStateParser.endElement(null, Project.Fields.no_ati_pref, null);
        ccStateParser.endElement(null, RPCCommonTags.PROJECT, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        final Project expectedProject = new Project();
        expectedProject.master_url = "Master URL";
        expectedProject.gui_urls.add(new GuiUrl("GUI URL Name", "GUI URL Description",
                                                "GUI URL URL"));
        expectedProject.no_ati_pref = true;

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
        ccStateParser.startElement(null, WorkunitsParser.WORKUNIT_TAG, null, null);
        ccStateParser.startElement(null, Workunit.Fields.name, null, null);
        ccStateParser.characters("Work unit".toCharArray(), 0, "Work unit".length());
        ccStateParser.endElement(null, Workunit.Fields.name, null);
        ccStateParser.startElement(null, Workunit.Fields.rsc_disk_bound, null, null);
        ccStateParser.characters("1.5".toCharArray(), 0, 3);
        ccStateParser.endElement(null, Workunit.Fields.rsc_disk_bound, null);
        ccStateParser.endElement(null, WorkunitsParser.WORKUNIT_TAG, null);
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null);

        final Workunit expectedWorkUnit = new Workunit();
        expectedWorkUnit.name = "Work unit";
        expectedWorkUnit.rsc_disk_bound = 1.5;
        expectedWorkUnit.project = new Project();

        expected.getWorkunits().add(expectedWorkUnit);

        assertEquals(expected, ccStateParser.getCcState());
    }
}