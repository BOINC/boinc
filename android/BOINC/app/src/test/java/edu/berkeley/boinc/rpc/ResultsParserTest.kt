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
import edu.berkeley.boinc.rpc.ResultsParser.Companion.parse
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
class ResultsParserTest {
    private lateinit var resultsParser: ResultsParser
    private lateinit var expected: Result
    @Before
    fun setUp() {
        resultsParser = ResultsParser()
        expected = Result()
    }

    @Test
    fun `When Rpc string is null then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        Assert.assertTrue(parse(null).isEmpty())
    }

    @Test
    @Throws(Exception::class)
    fun `When SAXException is thrown then expect empty list`() {
        PowerMockito.mockStatic(Xml::class.java)
        PowerMockito.mockStatic(Log::class.java)
        PowerMockito.doThrow(SAXException()).`when`(
            Xml::class.java, "parse", ArgumentMatchers.anyString(), ArgumentMatchers.any(
                ContentHandler::class.java
            )
        )
        Assert.assertTrue(parse("").isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When only start element is run then expect element started`() {
        resultsParser.startElement(null, "", null, null)
        Assert.assertTrue(resultsParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When both start element and end element are run then expect element not started`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        Assert.assertFalse(resultsParser.mElementStarted)
    }

    @Test
    @Throws(SAXException::class)
    fun `When 'localName' is empty then expect empty list`() {
        resultsParser.startElement(null, "", null, null)
        Assert.assertTrue(resultsParser.results.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result has no elements then expect empty list`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        Assert.assertTrue(resultsParser.results.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result has blank name then expect empty list`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters("".toCharArray(), 0, 0)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        Assert.assertTrue(resultsParser.results.isEmpty())
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result has non blank name then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result has name and active_task then expect active_task to be true`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        val results: List<Result> = resultsParser.results
        Assert.assertEquals(1, results.size.toLong())
        Assert.assertTrue(results[0].isActiveTask)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and work_unit_name then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.WU_NAME, null, null)
        resultsParser.characters("Work Unit".toCharArray(), 0, "Work Unit".length)
        resultsParser.endElement(null, Result.Fields.WU_NAME, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.workUnitName = "Work Unit"
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and project_url then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, PROJECT_URL, null, null)
        resultsParser.characters("Project URL".toCharArray(), 0, "Project URL".length)
        resultsParser.endElement(null, PROJECT_URL, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.projectURL = "Project URL"
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and invalid version_number then expect list with Result without version_number`() {
        PowerMockito.mockStatic(Log::class.java)
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.VERSION_NUM, null, null)
        resultsParser.characters("One".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.VERSION_NUM, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and version_number then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.VERSION_NUM, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.VERSION_NUM, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.versionNum = 1
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and ready_to_report is 0 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.READY_TO_REPORT, null, null)
        resultsParser.characters("0".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.READY_TO_REPORT, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isReadyToReport = false
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and ready_to_report is 1 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.READY_TO_REPORT, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.READY_TO_REPORT, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isReadyToReport = true
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and got_server_ack is 0 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.GOT_SERVER_ACK, null, null)
        resultsParser.characters("0".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.GOT_SERVER_ACK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.gotServerAck = false
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and got_server_ack is 1 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.GOT_SERVER_ACK, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.GOT_SERVER_ACK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.gotServerAck = true
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and final_cpu_time then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.FINAL_CPU_TIME, null, null)
        resultsParser.characters("1000000".toCharArray(), 0, "1000000".length)
        resultsParser.endElement(null, Result.Fields.FINAL_CPU_TIME, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.finalCPUTime = 1000000.0
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and final_elapsed_time then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.FINAL_ELAPSED_TIME, null, null)
        resultsParser.characters(TEN_BILLION_STR.toCharArray(), 0, TEN_BILLION_STR.length)
        resultsParser.endElement(null, Result.Fields.FINAL_ELAPSED_TIME, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.finalElapsedTime = TEN_BILLION.toDouble()
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and state then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.STATE, null, null)
        resultsParser.characters("1000".toCharArray(), 0, "1000".length)
        resultsParser.endElement(null, Result.Fields.STATE, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.state = 1000
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and report_deadline then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.REPORT_DEADLINE, null, null)
        resultsParser.characters(TEN_BILLION_STR.toCharArray(), 0, TEN_BILLION_STR.length)
        resultsParser.endElement(null, Result.Fields.REPORT_DEADLINE, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.reportDeadline = TEN_BILLION
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and received_time then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.RECEIVED_TIME, null, null)
        resultsParser.characters(TEN_BILLION_STR.toCharArray(), 0, TEN_BILLION_STR.length)
        resultsParser.endElement(null, Result.Fields.RECEIVED_TIME, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.receivedTime = TEN_BILLION
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and estimated_cpu_time_remaining then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ESTIMATED_CPU_TIME_REMAINING, null, null)
        resultsParser.characters(TEN_BILLION_STR.toCharArray(), 0, TEN_BILLION_STR.length)
        resultsParser.endElement(null, Result.Fields.ESTIMATED_CPU_TIME_REMAINING, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.estimatedCPUTimeRemaining = TEN_BILLION.toDouble()
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and exit_status then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.EXIT_STATUS, null, null)
        resultsParser.characters("0".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.EXIT_STATUS, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.exitStatus = 0
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and suspended_via_gui is 0 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.SUSPENDED_VIA_GUI, null, null)
        resultsParser.characters("0".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.SUSPENDED_VIA_GUI, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isSuspendedViaGUI = false
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and suspended_via_gui is 1 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.SUSPENDED_VIA_GUI, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.SUSPENDED_VIA_GUI, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isSuspendedViaGUI = true
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and project_suspended_via_gui is 0 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.PROJECT_SUSPENDED_VIA_GUI, null, null)
        resultsParser.characters("0".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.PROJECT_SUSPENDED_VIA_GUI, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isProjectSuspendedViaGUI = false
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and project_suspended_via_gui is 1 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.PROJECT_SUSPENDED_VIA_GUI, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.PROJECT_SUSPENDED_VIA_GUI, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isProjectSuspendedViaGUI = true
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for non active_task has name and resources then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.RESOURCES, null, null)
        resultsParser.characters("Resources".toCharArray(), 0, "Resources".length)
        resultsParser.endElement(null, Result.Fields.RESOURCES, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.resources = "Resources"
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and active_task_state then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK_STATE, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK_STATE, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.activeTaskState = 1
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and app_version_number then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.APP_VERSION_NUM, null, null)
        resultsParser.characters("10".toCharArray(), 0, 2)
        resultsParser.endElement(null, Result.Fields.APP_VERSION_NUM, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.appVersionNum = 10
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and scheduler_state then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.SCHEDULER_STATE, null, null)
        resultsParser.characters("1000".toCharArray(), 0, "1000".length)
        resultsParser.endElement(null, Result.Fields.SCHEDULER_STATE, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.schedulerState = 1000
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and checkpoint_cpu_time then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.CHECKPOINT_CPU_TIME, null, null)
        resultsParser.characters("150000".toCharArray(), 0, "150000".length)
        resultsParser.endElement(null, Result.Fields.CHECKPOINT_CPU_TIME, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.checkpointCPUTime = 150000.0
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and current_cpu_time then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.CURRENT_CPU_TIME, null, null)
        resultsParser.characters("100000".toCharArray(), 0, "100000".length)
        resultsParser.endElement(null, Result.Fields.CURRENT_CPU_TIME, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.currentCPUTime = 100000.0
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and fraction_done then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.FRACTION_DONE, null, null)
        resultsParser.characters("0.67".toCharArray(), 0, 4)
        resultsParser.endElement(null, Result.Fields.FRACTION_DONE, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.fractionDone = 0.67f
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and elapsed_time then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.ELAPSED_TIME, null, null)
        resultsParser.characters("2500".toCharArray(), 0, 4)
        resultsParser.endElement(null, Result.Fields.ELAPSED_TIME, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.elapsedTime = 2500.0
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and swap_size then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.SWAP_SIZE, null, null)
        resultsParser.characters("2500".toCharArray(), 0, 4)
        resultsParser.endElement(null, Result.Fields.SWAP_SIZE, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.swapSize = 2500.0
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and working_set_size_smoothed then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.WORKING_SET_SIZE_SMOOTHED, null, null)
        resultsParser.characters("2500".toCharArray(), 0, 4)
        resultsParser.endElement(null, Result.Fields.WORKING_SET_SIZE_SMOOTHED, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.workingSetSizeSmoothed = 2500.0
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and estimated_cpu_time_remaining then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.ESTIMATED_CPU_TIME_REMAINING, null, null)
        resultsParser.characters("1000".toCharArray(), 0, 4)
        resultsParser.endElement(null, Result.Fields.ESTIMATED_CPU_TIME_REMAINING, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.estimatedCPUTimeRemaining = 1000.0
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and supports_graphics is 0 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.SUPPORTS_GRAPHICS, null, null)
        resultsParser.characters("0".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.SUPPORTS_GRAPHICS, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.supportsGraphics = false
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and supports_graphics is 1 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.SUPPORTS_GRAPHICS, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.SUPPORTS_GRAPHICS, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.supportsGraphics = true
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and graphics_mode_acked then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.GRAPHICS_MODE_ACKED, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.GRAPHICS_MODE_ACKED, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.graphicsModeAcked = 1
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and too_large is 0 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.TOO_LARGE, null, null)
        resultsParser.characters("0".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.TOO_LARGE, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.isTooLarge = false
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and too_large is 1 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.TOO_LARGE, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.TOO_LARGE, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.isTooLarge = true
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and needs_shmem is 0 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.NEEDS_SHMEM, null, null)
        resultsParser.characters("0".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.NEEDS_SHMEM, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.needsShmem = false
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and needs_shmem is 1 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.NEEDS_SHMEM, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.NEEDS_SHMEM, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.needsShmem = true
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and edf_scheduled is 0 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.EDF_SCHEDULED, null, null)
        resultsParser.characters("0".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.EDF_SCHEDULED, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.isEdfScheduled = false
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and edf_scheduled is 1 then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.EDF_SCHEDULED, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.EDF_SCHEDULED, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.isEdfScheduled = true
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and pid then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.PID, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.PID, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.pid = 1
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and slot then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.SLOT, null, null)
        resultsParser.characters("1".toCharArray(), 0, 1)
        resultsParser.endElement(null, Result.Fields.SLOT, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.slot = 1
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and graphics_exec_path then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.GRAPHICS_EXEC_PATH, null, null)
        resultsParser.characters("/path/to/graphics".toCharArray(), 0, "/path/to/graphics".length)
        resultsParser.endElement(null, Result.Fields.GRAPHICS_EXEC_PATH, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.graphicsExecPath = "/path/to/graphics"
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    @Test
    @Throws(SAXException::class)
    fun `When Xml Result for active_task has name and slot_path then expect list with matching Result`() {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null)
        resultsParser.startElement(null, edu.berkeley.boinc.rpc.NAME, null, null)
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length)
        resultsParser.endElement(null, edu.berkeley.boinc.rpc.NAME, null)
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null)
        resultsParser.startElement(null, Result.Fields.SLOT_PATH, null, null)
        resultsParser.characters("/path/to/slot".toCharArray(), 0, "/path/to/slot".length)
        resultsParser.endElement(null, Result.Fields.SLOT_PATH, null)
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null)
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null)
        expected.name = NAME
        expected.isActiveTask = true
        expected.slotPath = "/path/to/slot"
        Assert.assertEquals(listOf(expected), resultsParser.results)
    }

    companion object {
        private const val TEN_BILLION = 10000000000L
        private const val TEN_BILLION_STR = TEN_BILLION.toString()
        private const val NAME = "Result"
    }
}
