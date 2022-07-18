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
import edu.berkeley.boinc.rpc.WorkUnitsParser.Companion.parse
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
class WorkUnitsParserTest {
    private lateinit var workUnitsParser: WorkUnitsParser
    private lateinit var expected: WorkUnit
    @Before
    fun setUp() {
        workUnitsParser = WorkUnitsParser()
        expected = WorkUnit()
    }

    @Test
    fun `When Rpc string is null then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        Assert.assertTrue(parse(null).isEmpty())
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown then expect null`() {
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertTrue(parse(null).isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When only start element is run then expect element started`() {
        workUnitsParser.startElement(null, "", null, null)
        Assert.assertTrue(workUnitsParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When both start element and end element are run then expect element not started`() {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null)
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null)
        Assert.assertFalse(workUnitsParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect empty list`() {
        workUnitsParser.startElement(null, "", null, null)
        Assert.assertTrue(workUnitsParser.workUnits.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml WorkUnit has no elements then expect empty list`() {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null)
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null)
        Assert.assertTrue(workUnitsParser.workUnits.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml WorkUnit has blank name then expect empty list`() {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null)
        workUnitsParser.startElement(null, NAME, null, null)
        workUnitsParser.characters("".toCharArray(), 0, 0)
        workUnitsParser.endElement(null, NAME, null)
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null)
        Assert.assertTrue(workUnitsParser.workUnits.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml WorkUnit has only name then expect list with matching WorkUnit`() {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null)
        workUnitsParser.startElement(null, NAME, null, null)
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length)
        workUnitsParser.endElement(null, NAME, null)
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null)
        expected.name = WORK_UNIT_NAME
        Assert.assertEquals(listOf(expected), workUnitsParser.workUnits)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml WorkUnit has only name and app_name then expect list with matching WorkUnit`() {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null)
        workUnitsParser.startElement(null, NAME, null, null)
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length)
        workUnitsParser.endElement(null, NAME, null)
        workUnitsParser.startElement(null, WorkUnit.Fields.APP_NAME, null, null)
        workUnitsParser.characters("App".toCharArray(), 0, 3)
        workUnitsParser.endElement(null, WorkUnit.Fields.APP_NAME, null)
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null)
        expected.name = WORK_UNIT_NAME
        expected.appName = "App"
        Assert.assertEquals(listOf(expected), workUnitsParser.workUnits)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml WorkUnit has name and invalid version_num then expect list with WorkUnit with only name`() {
        PowerMockito.mockStatic(Log::class.java)
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null)
        workUnitsParser.startElement(null, NAME, null, null)
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length)
        workUnitsParser.endElement(null, NAME, null)
        workUnitsParser.startElement(null, WorkUnit.Fields.VERSION_NUM, null, null)
        workUnitsParser.characters("One".toCharArray(), 0, 3)
        workUnitsParser.endElement(null, WorkUnit.Fields.VERSION_NUM, null)
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null)
        expected.name = WORK_UNIT_NAME
        Assert.assertEquals(listOf(expected), workUnitsParser.workUnits)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml WorkUnit has name and version_num then expect list with WorkUnit with only name`() {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null)
        workUnitsParser.startElement(null, NAME, null, null)
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length)
        workUnitsParser.endElement(null, NAME, null)
        workUnitsParser.startElement(null, WorkUnit.Fields.VERSION_NUM, null, null)
        workUnitsParser.characters("1".toCharArray(), 0, 1)
        workUnitsParser.endElement(null, WorkUnit.Fields.VERSION_NUM, null)
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null)
        expected.name = WORK_UNIT_NAME
        expected.versionNum = 1
        Assert.assertEquals(listOf(expected), workUnitsParser.workUnits)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml WorkUnit has name and rsc_floating_point_ops_est then expect list with WorkUnit with only name`() {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null)
        workUnitsParser.startElement(null, NAME, null, null)
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length)
        workUnitsParser.endElement(null, NAME, null)
        workUnitsParser.startElement(null, WorkUnit.Fields.RSC_FPOPS_EST, null, null)
        workUnitsParser.characters("1.5".toCharArray(), 0, 3)
        workUnitsParser.endElement(null, WorkUnit.Fields.RSC_FPOPS_EST, null)
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null)
        expected.name = WORK_UNIT_NAME
        expected.rscFloatingPointOpsEst = 1.5
        Assert.assertEquals(listOf(expected), workUnitsParser.workUnits)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml WorkUnit has name and rsc_floating_point_ops_bound then expect list with WorkUnit with only name`() {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null)
        workUnitsParser.startElement(null, NAME, null, null)
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length)
        workUnitsParser.endElement(null, NAME, null)
        workUnitsParser.startElement(null, WorkUnit.Fields.RSC_FPOPS_BOUND, null, null)
        workUnitsParser.characters("1.5".toCharArray(), 0, 3)
        workUnitsParser.endElement(null, WorkUnit.Fields.RSC_FPOPS_BOUND, null)
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null)
        expected.name = WORK_UNIT_NAME
        expected.rscFloatingPointOpsBound = 1.5
        Assert.assertEquals(listOf(expected), workUnitsParser.workUnits)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml WorkUnit has name and rsc_memory_bound then expect list with WorkUnit with only name`() {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null)
        workUnitsParser.startElement(null, NAME, null, null)
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length)
        workUnitsParser.endElement(null, NAME, null)
        workUnitsParser.startElement(null, WorkUnit.Fields.RSC_MEMORY_BOUND, null, null)
        workUnitsParser.characters("1.5".toCharArray(), 0, 3)
        workUnitsParser.endElement(null, WorkUnit.Fields.RSC_MEMORY_BOUND, null)
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null)
        expected.name = WORK_UNIT_NAME
        expected.rscMemoryBound = 1.5
        Assert.assertEquals(listOf(expected), workUnitsParser.workUnits)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml WorkUnit has name and rsc_disk_bound then expect list with WorkUnit with only name`() {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null)
        workUnitsParser.startElement(null, NAME, null, null)
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length)
        workUnitsParser.endElement(null, NAME, null)
        workUnitsParser.startElement(null, WorkUnit.Fields.RSC_DISK_BOUND, null, null)
        workUnitsParser.characters("1.5".toCharArray(), 0, 3)
        workUnitsParser.endElement(null, WorkUnit.Fields.RSC_DISK_BOUND, null)
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null)
        expected.name = WORK_UNIT_NAME
        expected.rscDiskBound = 1.5
        Assert.assertEquals(listOf(expected), workUnitsParser.workUnits)
    }

    companion object {
        private const val WORK_UNIT_NAME = "Work Unit"
    }
}
