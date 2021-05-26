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
import edu.berkeley.boinc.rpc.CcStateParser.Companion.parse
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
class CcStateParserTest {
    private lateinit var ccStateParser: CcStateParser
    private lateinit var expected: CcState
    @Before
    fun setUp() {
        ccStateParser = CcStateParser()
        expected = CcState()
    }

    @Test
    fun `When Rpc string is null then expect default CcState`() {
        PowerMockito.mockStatic(Xml::class.java)
        Assert.assertEquals(expected, parse(null))
    }

    @Test
    fun `When Rpc string is empty then expect default CcState`() {
        PowerMockito.mockStatic(Xml::class.java)
        Assert.assertEquals(expected, parse(""))
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
    fun `When 'localName' is empty then expect element not started`() {
        ccStateParser.startElement(null, "", null, null)
        Assert.assertFalse(ccStateParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is client_state tag then expect element not started`() {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null)
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null)
        Assert.assertFalse(ccStateParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect default CcState`() {
        ccStateParser.startElement(null, "", null, null)
        Assert.assertEquals(expected, ccStateParser.ccState)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is client_state tag then expect default CcState`() {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null)
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null)
        Assert.assertEquals(expected, ccStateParser.ccState)
    }

    @Test
    @Throws(SAXException::class)
    fun `When entering Xml CcState then expect empty CcState lists`() {
        val ccState = ccStateParser.ccState
        ccState.apps.add(App())
        ccState.appVersions.add(AppVersion())
        ccState.projects.add(Project())
        ccState.results.add(Result())
        ccState.workUnits.add(WorkUnit())
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null)
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null)
        Assert.assertTrue(ccState.apps.isEmpty())
        Assert.assertTrue(ccState.appVersions.isEmpty())
        Assert.assertTrue(ccState.projects.isEmpty())
        Assert.assertTrue(ccState.results.isEmpty())
        Assert.assertTrue(ccState.workUnits.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcState has app then expect matching CcState`() {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null)
        ccStateParser.startElement(null, AppsParser.APP_TAG, null, null)
        ccStateParser.startElement(null, NAME, null, null)
        ccStateParser.characters("App".toCharArray(), 0, 3)
        ccStateParser.endElement(null, NAME, null)
        ccStateParser.startElement(null, USER_FRIENDLY_NAME, null, null)
        ccStateParser.characters(
            "App (Friendly Name)".toCharArray(),
            0,
            "App (Friendly Name)".length
        )
        ccStateParser.endElement(null, USER_FRIENDLY_NAME, null)
        ccStateParser.startElement(null, NON_CPU_INTENSIVE, null, null)
        ccStateParser.characters("1".toCharArray(), 0, 1)
        ccStateParser.endElement(null, NON_CPU_INTENSIVE, null)
        ccStateParser.endElement(null, AppsParser.APP_TAG, null)
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null)
        val expectedApp = App("App", "App (Friendly Name)", 1)
        expectedApp.project = Project()
        expected.apps.add(expectedApp)
        Assert.assertEquals(expected, ccStateParser.ccState)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcState has app_version then expect matching CcState`() {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null)
        ccStateParser.startElement(null, AppVersionsParser.APP_VERSION_TAG, null, null)
        ccStateParser.startElement(null, AppVersion.Fields.APP_NAME, null, null)
        ccStateParser.characters("App".toCharArray(), 0, 3)
        ccStateParser.endElement(null, AppVersion.Fields.APP_NAME, null)
        ccStateParser.startElement(null, AppVersion.Fields.VERSION_NUM, null, null)
        ccStateParser.characters("1".toCharArray(), 0, 1)
        ccStateParser.endElement(null, AppVersion.Fields.VERSION_NUM, null)
        ccStateParser.endElement(null, AppVersionsParser.APP_VERSION_TAG, null)
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null)
        val expectedAppVersion = AppVersion("App", 1)
        expectedAppVersion.project = Project()
        expected.appVersions.add(expectedAppVersion)
        Assert.assertEquals(expected, ccStateParser.ccState)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcState has host_info then expect matching CcState`() {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null)
        ccStateParser.startElement(null, HostInfoParser.HOST_INFO_TAG, null, null)
        ccStateParser.startElement(null, HostInfo.Fields.TIMEZONE, null, null)
        ccStateParser.characters("1".toCharArray(), 0, 1)
        ccStateParser.endElement(null, HostInfo.Fields.TIMEZONE, null)
        ccStateParser.startElement(null, HostInfo.Fields.DOMAIN_NAME, null, null)
        ccStateParser.characters("Domain Name".toCharArray(), 0, "Domain Name".length)
        ccStateParser.endElement(null, HostInfo.Fields.DOMAIN_NAME, null)
        ccStateParser.startElement(null, HostInfo.Fields.IP_ADDR, null, null)
        ccStateParser.characters("IP Address".toCharArray(), 0, "IP Address".length)
        ccStateParser.endElement(null, HostInfo.Fields.IP_ADDR, null)
        ccStateParser.startElement(null, HostInfo.Fields.HOST_CPID, null, null)
        ccStateParser.characters("Host CPID".toCharArray(), 0, "Host CPID".length)
        ccStateParser.endElement(null, HostInfo.Fields.HOST_CPID, null)
        ccStateParser.startElement(null, HostInfo.Fields.P_NCPUS, null, null)
        ccStateParser.characters("1".toCharArray(), 0, 1)
        ccStateParser.endElement(null, HostInfo.Fields.P_NCPUS, null)
        ccStateParser.startElement(null, HostInfo.Fields.P_VENDOR, null, null)
        ccStateParser.characters("Vendor".toCharArray(), 0, "Vendor".length)
        ccStateParser.endElement(null, HostInfo.Fields.P_VENDOR, null)
        ccStateParser.startElement(null, HostInfo.Fields.P_MODEL, null, null)
        ccStateParser.characters("Model".toCharArray(), 0, "Model".length)
        ccStateParser.endElement(null, HostInfo.Fields.P_MODEL, null)
        ccStateParser.startElement(null, HostInfo.Fields.P_FEATURES, null, null)
        ccStateParser.characters("Features".toCharArray(), 0, "Features".length)
        ccStateParser.endElement(null, HostInfo.Fields.P_FEATURES, null)
        ccStateParser.startElement(null, HostInfo.Fields.P_FPOPS, null, null)
        ccStateParser.characters("1.5".toCharArray(), 0, 3)
        ccStateParser.endElement(null, HostInfo.Fields.P_FPOPS, null)
        ccStateParser.startElement(null, HostInfo.Fields.P_IOPS, null, null)
        ccStateParser.characters("1.5".toCharArray(), 0, 3)
        ccStateParser.endElement(null, HostInfo.Fields.P_IOPS, null)
        ccStateParser.startElement(null, HostInfo.Fields.P_MEMBW, null, null)
        ccStateParser.characters("1.5".toCharArray(), 0, 3)
        ccStateParser.endElement(null, HostInfo.Fields.P_MEMBW, null)
        ccStateParser.startElement(null, HostInfo.Fields.P_CALCULATED, null, null)
        ccStateParser.characters("0".toCharArray(), 0, 1)
        ccStateParser.endElement(null, HostInfo.Fields.P_CALCULATED, null)
        ccStateParser.startElement(null, HostInfo.Fields.PRODUCT_NAME, null, null)
        ccStateParser.characters("Product Name".toCharArray(), 0, "Product Name".length)
        ccStateParser.endElement(null, HostInfo.Fields.PRODUCT_NAME, null)
        ccStateParser.startElement(null, HostInfo.Fields.M_NBYTES, null, null)
        ccStateParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length)
        ccStateParser.endElement(null, HostInfo.Fields.M_NBYTES, null)
        ccStateParser.startElement(null, HostInfo.Fields.M_CACHE, null, null)
        ccStateParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length)
        ccStateParser.endElement(null, HostInfo.Fields.M_CACHE, null)
        ccStateParser.startElement(null, HostInfo.Fields.M_SWAP, null, null)
        ccStateParser.characters(BYTES_IN_1_GB_STR.toCharArray(), 0, BYTES_IN_1_GB_STR.length)
        ccStateParser.endElement(null, HostInfo.Fields.M_SWAP, null)
        ccStateParser.startElement(null, HostInfo.Fields.D_TOTAL, null, null)
        ccStateParser.characters(TOTAL_SPACE_STR.toCharArray(), 0, TOTAL_SPACE_STR.length)
        ccStateParser.endElement(null, HostInfo.Fields.D_TOTAL, null)
        ccStateParser.startElement(null, HostInfo.Fields.D_FREE, null, null)
        ccStateParser.characters(FREE_SPACE_STR.toCharArray(), 0, FREE_SPACE_STR.length)
        ccStateParser.endElement(null, HostInfo.Fields.D_FREE, null)
        ccStateParser.startElement(null, HostInfo.Fields.OS_NAME, null, null)
        ccStateParser.characters("Android".toCharArray(), 0, "Android".length)
        ccStateParser.endElement(null, HostInfo.Fields.OS_NAME, null)
        ccStateParser.startElement(null, HostInfo.Fields.OS_VERSION, null, null)
        ccStateParser.characters("Q".toCharArray(), 0, "Q".length)
        ccStateParser.endElement(null, HostInfo.Fields.OS_VERSION, null)
        ccStateParser.startElement(null, HostInfo.Fields.VIRTUALBOX_VERSION, null, null)
        ccStateParser.characters("6.0.18".toCharArray(), 0, "6.0.18".length)
        ccStateParser.endElement(null, HostInfo.Fields.VIRTUALBOX_VERSION, null)
        ccStateParser.endElement(null, HostInfoParser.HOST_INFO_TAG, null)
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null)
        val expectedHostInfo = HostInfo()
        expectedHostInfo.timezone = 1
        expectedHostInfo.domainName = "Domain Name"
        expectedHostInfo.ipAddress = "IP Address"
        expectedHostInfo.hostCpid = "Host CPID"
        expectedHostInfo.noOfCPUs = 1
        expectedHostInfo.cpuVendor = "Vendor"
        expectedHostInfo.cpuModel = "Model"
        expectedHostInfo.cpuFeatures = "Features"
        expectedHostInfo.cpuFloatingPointOps = 1.5
        expectedHostInfo.cpuIntegerOps = 1.5
        expectedHostInfo.cpuMembw = 1.5
        expectedHostInfo.cpuCalculated = 0L
        expectedHostInfo.productName = "Product Name"
        expectedHostInfo.memoryInBytes = BYTES_IN_1_GB.toDouble()
        expectedHostInfo.memoryCache = BYTES_IN_1_GB.toDouble()
        expectedHostInfo.memorySwap = BYTES_IN_1_GB.toDouble()
        expectedHostInfo.totalDiskSpace = TOTAL_SPACE.toDouble()
        expectedHostInfo.freeDiskSpace = FREE_SPACE.toDouble()
        expectedHostInfo.osName = "Android"
        expectedHostInfo.osVersion = "Q"
        expectedHostInfo.virtualBoxVersion = "6.0.18"
        expected.hostInfo = expectedHostInfo
        Assert.assertEquals(expected, ccStateParser.ccState)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcState has project then expect matching CcState`() {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null)
        ccStateParser.startElement(null, PROJECT, null, null)
        ccStateParser.startElement(null, MASTER_URL, null, null)
        ccStateParser.characters("Master URL".toCharArray(), 0, "Master URL".length)
        ccStateParser.endElement(null, MASTER_URL, null)
        ccStateParser.startElement(null, GUI_URL, null, null)
        ccStateParser.startElement(null, NAME, null, null)
        ccStateParser.characters("GUI URL Name".toCharArray(), 0, "GUI URL Name".length)
        ccStateParser.endElement(null, NAME, null)
        ccStateParser.startElement(null, DESCRIPTION, null, null)
        ccStateParser.characters(
            "GUI URL Description".toCharArray(),
            0,
            "GUI URL Description".length
        )
        ccStateParser.endElement(null, DESCRIPTION, null)
        ccStateParser.startElement(null, URL, null, null)
        ccStateParser.characters("GUI URL URL".toCharArray(), 0, "GUI URL URL".length)
        ccStateParser.endElement(null, URL, null)
        ccStateParser.endElement(null, GUI_URL, null)
        ccStateParser.startElement(null, Project.Fields.NO_ATI_PREF, null, null)
        ccStateParser.characters("1".toCharArray(), 0, 1)
        ccStateParser.endElement(null, Project.Fields.NO_ATI_PREF, null)
        ccStateParser.endElement(null, PROJECT, null)
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null)
        val expectedProject = Project()
        expectedProject.masterURL = "Master URL"
        expectedProject.guiURLs.add(
            GuiUrl(
                "GUI URL Name", "GUI URL Description",
                "GUI URL URL"
            )
        )
        expectedProject.noATIPref = true
        expected.projects.add(expectedProject)
        Assert.assertEquals(expected, ccStateParser.ccState)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcState has result then expect matching CcState`() {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null)
        ccStateParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        ccStateParser.startElement(null, NAME, null, null)
        ccStateParser.characters("Result".toCharArray(), 0, "Result".length)
        ccStateParser.endElement(null, NAME, null)
        ccStateParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        ccStateParser.startElement(null, Result.Fields.SLOT_PATH, null, null)
        ccStateParser.characters("/path/to/slot".toCharArray(), 0, "/path/to/slot".length)
        ccStateParser.endElement(null, Result.Fields.SLOT_PATH, null)
        ccStateParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        ccStateParser.endElement(null, ResultsParser.RESULT_TAG, null)
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null)
        val expectedResult = Result()
        expectedResult.isActiveTask = true
        expectedResult.name = "Result"
        expectedResult.project = Project()
        expectedResult.slotPath = "/path/to/slot"
        expected.results.add(expectedResult)
        Assert.assertEquals(expected, ccStateParser.ccState)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcState has version_info with invalid major_version then expect CcState with default major_version`() {
        PowerMockito.mockStatic(Log::class.java)
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null)
        ccStateParser.startElement(null, CcStateParser.CORE_CLIENT_MAJOR_VERSION_TAG, null, null)
        ccStateParser.characters("One".toCharArray(), 0, 3)
        ccStateParser.endElement(null, CcStateParser.CORE_CLIENT_MAJOR_VERSION_TAG, null)
        ccStateParser.startElement(null, CcStateParser.CORE_CLIENT_MINOR_VERSION_TAG, null, null)
        ccStateParser.characters("1".toCharArray(), 0, 1)
        ccStateParser.endElement(null, CcStateParser.CORE_CLIENT_MINOR_VERSION_TAG, null)
        ccStateParser.startElement(null, CcStateParser.CORE_CLIENT_RELEASE_TAG, null, null)
        ccStateParser.characters("1".toCharArray(), 0, 1)
        ccStateParser.endElement(null, CcStateParser.CORE_CLIENT_RELEASE_TAG, null)
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null)
        expected.versionInfo = VersionInfo(0, 1, 1)
        Assert.assertEquals(expected, ccStateParser.ccState)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcState has version_info then expect matching CcState`() {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null)
        ccStateParser.startElement(null, CcStateParser.CORE_CLIENT_MAJOR_VERSION_TAG, null, null)
        ccStateParser.characters("1".toCharArray(), 0, 1)
        ccStateParser.endElement(null, CcStateParser.CORE_CLIENT_MAJOR_VERSION_TAG, null)
        ccStateParser.startElement(null, CcStateParser.CORE_CLIENT_MINOR_VERSION_TAG, null, null)
        ccStateParser.characters("1".toCharArray(), 0, 1)
        ccStateParser.endElement(null, CcStateParser.CORE_CLIENT_MINOR_VERSION_TAG, null)
        ccStateParser.startElement(null, CcStateParser.CORE_CLIENT_RELEASE_TAG, null, null)
        ccStateParser.characters("1".toCharArray(), 0, 1)
        ccStateParser.endElement(null, CcStateParser.CORE_CLIENT_RELEASE_TAG, null)
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null)
        expected.versionInfo = VersionInfo(1, 1, 1)
        Assert.assertEquals(expected, ccStateParser.ccState)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml CcState has work_unit then expect matching CcState`() {
        ccStateParser.startElement(null, CcStateParser.CLIENT_STATE_TAG, null, null)
        ccStateParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null)
        ccStateParser.startElement(null, NAME, null, null)
        ccStateParser.characters("Work unit".toCharArray(), 0, "Work unit".length)
        ccStateParser.endElement(null, NAME, null)
        ccStateParser.startElement(null, WorkUnit.Fields.RSC_DISK_BOUND, null, null)
        ccStateParser.characters("1.5".toCharArray(), 0, 3)
        ccStateParser.endElement(null, WorkUnit.Fields.RSC_DISK_BOUND, null)
        ccStateParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null)
        ccStateParser.endElement(null, CcStateParser.CLIENT_STATE_TAG, null)
        val expectedWorkUnit = WorkUnit()
        expectedWorkUnit.name = "Work unit"
        expectedWorkUnit.rscDiskBound = 1.5
        expectedWorkUnit.project = Project()
        expected.workUnits.add(expectedWorkUnit)
        Assert.assertEquals(expected, ccStateParser.ccState)
    }

    companion object {
        /* Project test values */
        private const val BYTES_IN_1_GB = 1073741824
        private const val BYTES_IN_1_GB_STR = BYTES_IN_1_GB.toString()
        private const val TOTAL_SPACE = BYTES_IN_1_GB * 16L
        private const val TOTAL_SPACE_STR = TOTAL_SPACE.toString()
        private const val FREE_SPACE = BYTES_IN_1_GB * 12L
        private const val FREE_SPACE_STR = FREE_SPACE.toString()
    }
}
