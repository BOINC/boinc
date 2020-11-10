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
import org.xml.sax.ContentHandler;
import org.xml.sax.SAXException;

import java.util.Collections;
import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.doThrow;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class ResultsParserTest {
    private static final long TEN_BILLION = 10_000_000_000L;
    private static final String TEN_BILLION_STR = Long.toString(TEN_BILLION);

    private static final String NAME = "Result";

    private ResultsParser resultsParser;
    private Result expected;

    @Before
    public void setUp() {
        resultsParser = new ResultsParser();
        expected = new Result();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectEmptyList() {
        mockStatic(Xml.class);

        assertTrue(ResultsParser.parse(null).isEmpty());
    }

    @Test
    public void testParse_whenSAXExceptionIsThrown_thenExpectEmptyList() throws Exception {
        mockStatic(Xml.class);

        doThrow(new SAXException()).when(Xml.class, "parse", anyString(), any(ContentHandler.class));

        assertTrue(ResultsParser.parse("").isEmpty());
    }

    @Test
    public void testParser_whenOnlyStartElementIsRun_thenExpectElementStarted() throws SAXException {
        resultsParser.startElement(null, "", null, null);

        assertTrue(resultsParser.mElementStarted);
    }

    @Test
    public void testParser_whenBothStartElementAndEndElementAreRun_thenExpectElementNotStarted()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        assertFalse(resultsParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectEmptyList() throws SAXException {
        resultsParser.startElement(null, "", null, null);

        assertTrue(resultsParser.getResults().isEmpty());
    }

    @Test
    public void testParser_whenXmlResultHasNoElements_thenExpectEmptyList()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        assertTrue(resultsParser.getResults().isEmpty());
    }

    @Test
    public void testParser_whenXmlResultHasBlankName_thenExpectEmptyList()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters("".toCharArray(), 0, 0);
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        assertTrue(resultsParser.getResults().isEmpty());
    }

    @Test
    public void testParser_whenXmlResultHasNonBlankName_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultHasNameAndActiveTask_thenExpectActiveTaskToBeTrue()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        final List<Result> results = resultsParser.getResults();

        assertEquals(1, results.size());
        assertTrue(results.get(0).isActiveTask());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndWorkUnitName_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.WU_NAME, null, null);
        resultsParser.characters("Work Unit".toCharArray(), 0, "Work Unit".length());
        resultsParser.endElement(null, Result.Fields.WU_NAME, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setWorkUnitName("Work Unit");

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndProjectUrl_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, RPCCommonTags.PROJECT_URL, null, null);
        resultsParser.characters("Project URL".toCharArray(), 0, "Project URL".length());
        resultsParser.endElement(null, RPCCommonTags.PROJECT_URL, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setProjectURL("Project URL");

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndInvalidVersionNumber_thenExpectListWithResultWithoutVersionNumber()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.VERSION_NUM, null, null);
        resultsParser.characters("One".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.VERSION_NUM, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndVersionNumber_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.VERSION_NUM, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.VERSION_NUM, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setVersionNum(1);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndReadyToReportIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.READY_TO_REPORT, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.READY_TO_REPORT, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setReadyToReport(false);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndReadyToReportIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.READY_TO_REPORT, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.READY_TO_REPORT, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setReadyToReport(true);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndGotServerAckIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.GOT_SERVER_ACK, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.GOT_SERVER_ACK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setGotServerAck(false);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndGotServerAckIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.GOT_SERVER_ACK, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.GOT_SERVER_ACK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setGotServerAck(true);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndFinalCpuTime_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.FINAL_CPU_TIME, null, null);
        resultsParser.characters("1000000".toCharArray(), 0, "1000000".length());
        resultsParser.endElement(null, Result.Fields.FINAL_CPU_TIME, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setFinalCPUTime(1_000_000);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndFinalElapsedTime_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.FINAL_ELAPSED_TIME, null, null);
        resultsParser.characters(TEN_BILLION_STR.toCharArray(), 0, TEN_BILLION_STR.length());
        resultsParser.endElement(null, Result.Fields.FINAL_ELAPSED_TIME, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setFinalElapsedTime(TEN_BILLION);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndState_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.STATE, null, null);
        resultsParser.characters("1000".toCharArray(), 0, "1000".length());
        resultsParser.endElement(null, Result.Fields.STATE, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setState(1000);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndReportDeadline_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.REPORT_DEADLINE, null, null);
        resultsParser.characters(TEN_BILLION_STR.toCharArray(), 0, TEN_BILLION_STR.length());
        resultsParser.endElement(null, Result.Fields.REPORT_DEADLINE, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setReportDeadline(TEN_BILLION);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndReceivedTime_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.RECEIVED_TIME, null, null);
        resultsParser.characters(TEN_BILLION_STR.toCharArray(), 0, TEN_BILLION_STR.length());
        resultsParser.endElement(null, Result.Fields.RECEIVED_TIME, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setReceivedTime(TEN_BILLION);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndEstimatedCpuTimeRemaining_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ESTIMATED_CPU_TIME_REMAINING, null, null);
        resultsParser.characters(TEN_BILLION_STR.toCharArray(), 0, TEN_BILLION_STR.length());
        resultsParser.endElement(null, Result.Fields.ESTIMATED_CPU_TIME_REMAINING, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setEstimatedCPUTimeRemaining(TEN_BILLION);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndExitStatus_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.EXIT_STATUS, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.EXIT_STATUS, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setExitStatus(0);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndSuspendedViaGuiIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.SUSPENDED_VIA_GUI, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.SUSPENDED_VIA_GUI, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setSuspendedViaGUI(false);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndSuspendedViaGuiIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.SUSPENDED_VIA_GUI, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.SUSPENDED_VIA_GUI, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setSuspendedViaGUI(true);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndProjectSuspendedViaGuiIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.PROJECT_SUSPENDED_VIA_GUI, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.PROJECT_SUSPENDED_VIA_GUI, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setProjectSuspendedViaGUI(false);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndProjectSuspendedViaGuiIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.PROJECT_SUSPENDED_VIA_GUI, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.PROJECT_SUSPENDED_VIA_GUI, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setProjectSuspendedViaGUI(true);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndResources_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.RESOURCES, null, null);
        resultsParser.characters("Resources".toCharArray(), 0, "Resources".length());
        resultsParser.endElement(null, Result.Fields.RESOURCES, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setResources("Resources");

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndActiveTaskState_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK_STATE, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK_STATE, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setActiveTaskState(1);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndAppVersionNumber_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.APP_VERSION_NUM, null, null);
        resultsParser.characters("10".toCharArray(), 0, 2);
        resultsParser.endElement(null, Result.Fields.APP_VERSION_NUM, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setAppVersionNum(10);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndSchedulerState_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.SCHEDULER_STATE, null, null);
        resultsParser.characters("1000".toCharArray(), 0, "1000".length());
        resultsParser.endElement(null, Result.Fields.SCHEDULER_STATE, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setSchedulerState(1000);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndCheckpointCpuTime_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.CHECKPOINT_CPU_TIME, null, null);
        resultsParser.characters("150000".toCharArray(), 0, "150000".length());
        resultsParser.endElement(null, Result.Fields.CHECKPOINT_CPU_TIME, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setCheckpointCPUTime(150_000);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndCurrentCpuTime_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.CURRENT_CPU_TIME, null, null);
        resultsParser.characters("100000".toCharArray(), 0, "100000".length());
        resultsParser.endElement(null, Result.Fields.CURRENT_CPU_TIME, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setCurrentCPUTime(100_000);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndFractionDone_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.FRACTION_DONE, null, null);
        resultsParser.characters("0.67".toCharArray(), 0, 4);
        resultsParser.endElement(null, Result.Fields.FRACTION_DONE, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setFractionDone(0.67f);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndElapsedTime_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.ELAPSED_TIME, null, null);
        resultsParser.characters("2500".toCharArray(), 0, 4);
        resultsParser.endElement(null, Result.Fields.ELAPSED_TIME, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setElapsedTime(2500);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndSwapSize_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.SWAP_SIZE, null, null);
        resultsParser.characters("2500".toCharArray(), 0, 4);
        resultsParser.endElement(null, Result.Fields.SWAP_SIZE, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setSwapSize(2500);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndWorkingSetSizeSmoothed_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.WORKING_SET_SIZE_SMOOTHED, null, null);
        resultsParser.characters("2500".toCharArray(), 0, 4);
        resultsParser.endElement(null, Result.Fields.WORKING_SET_SIZE_SMOOTHED, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setWorkingSetSizeSmoothed(2500);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndEstimatedCpuTimeRemaining_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.ESTIMATED_CPU_TIME_REMAINING, null, null);
        resultsParser.characters("1000".toCharArray(), 0, 4);
        resultsParser.endElement(null, Result.Fields.ESTIMATED_CPU_TIME_REMAINING, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setEstimatedCPUTimeRemaining(1000);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndSupportsGraphicsIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.SUPPORTS_GRAPHICS, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.SUPPORTS_GRAPHICS, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setSupportsGraphics(false);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndSupportsGraphicsIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.SUPPORTS_GRAPHICS, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.SUPPORTS_GRAPHICS, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setSupportsGraphics(true);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndGraphicsModeAcked_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.GRAPHICS_MODE_ACKED, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.GRAPHICS_MODE_ACKED, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setGraphicsModeAcked(1);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndTooLargeIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.TOO_LARGE, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.TOO_LARGE, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setTooLarge(false);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndTooLargeIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.TOO_LARGE, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.TOO_LARGE, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setTooLarge(true);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndNeedsShMemIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.NEEDS_SHMEM, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.NEEDS_SHMEM, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setNeedsShmem(false);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndNeedsShmemIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.NEEDS_SHMEM, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.NEEDS_SHMEM, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setNeedsShmem(true);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndEdfScheduledIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.EDF_SCHEDULED, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.EDF_SCHEDULED, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setEdfScheduled(false);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndEdfScheduledIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.EDF_SCHEDULED, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.EDF_SCHEDULED, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setEdfScheduled(true);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndPid_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.PID, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.PID, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setPid(1);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndSlot_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.SLOT, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.SLOT, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setSlot(1);

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndGraphicsExecPath_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.GRAPHICS_EXEC_PATH, null, null);
        resultsParser.characters("/path/to/graphics".toCharArray(), 0, "/path/to/graphics".length());
        resultsParser.endElement(null, Result.Fields.GRAPHICS_EXEC_PATH, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setGraphicsExecPath("/path/to/graphics");

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndSlotPath_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ACTIVE_TASK, null, null);
        resultsParser.startElement(null, Result.Fields.SLOT_PATH, null, null);
        resultsParser.characters("/path/to/slot".toCharArray(), 0, "/path/to/slot".length());
        resultsParser.endElement(null, Result.Fields.SLOT_PATH, null);
        resultsParser.endElement(null, Result.Fields.ACTIVE_TASK, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.setName(NAME);
        expected.setActiveTask(true);
        expected.setSlotPath("/path/to/slot");

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }
}
