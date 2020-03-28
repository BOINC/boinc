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

import java.util.Collections;
import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
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

        assertEquals(Collections.emptyList(), ResultsParser.parse(null));
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

        expected.name = NAME;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultHasNameAndActiveTask_thenExpectActiveTaskToBeTrue()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        final List<Result> results = resultsParser.getResults();

        assertEquals(1, results.size());
        assertTrue(results.get(0).active_task);
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndWorkUnitName_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.wu_name, null, null);
        resultsParser.characters("Work Unit".toCharArray(), 0, "Work Unit".length());
        resultsParser.endElement(null, Result.Fields.wu_name, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.wu_name = "Work Unit";

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

        expected.name = NAME;
        expected.project_url = "Project URL";

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndInvalidVersionNumber_thenExpectListWithResultWithoutVersionNumber()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.version_num, null, null);
        resultsParser.characters("One".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.version_num, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndVersionNumber_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.version_num, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.version_num, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.version_num = 1;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndReadyToReportIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ready_to_report, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.ready_to_report, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.ready_to_report = false;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndReadyToReportIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.ready_to_report, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.ready_to_report, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.ready_to_report = true;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndGotServerAckIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.got_server_ack, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.got_server_ack, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.got_server_ack = false;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndGotServerAckIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.got_server_ack, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.got_server_ack, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.got_server_ack = true;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndFinalCpuTime_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.final_cpu_time, null, null);
        resultsParser.characters("1000000".toCharArray(), 0, "1000000".length());
        resultsParser.endElement(null, Result.Fields.final_cpu_time, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.final_cpu_time = 1_000_000;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndFinalElapsedTime_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.final_elapsed_time, null, null);
        resultsParser.characters(TEN_BILLION_STR.toCharArray(), 0, TEN_BILLION_STR.length());
        resultsParser.endElement(null, Result.Fields.final_elapsed_time, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.final_elapsed_time = TEN_BILLION;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndState_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.state, null, null);
        resultsParser.characters("1000".toCharArray(), 0, "1000".length());
        resultsParser.endElement(null, Result.Fields.state, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.state = 1000;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndReportDeadline_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.report_deadline, null, null);
        resultsParser.characters(TEN_BILLION_STR.toCharArray(), 0, TEN_BILLION_STR.length());
        resultsParser.endElement(null, Result.Fields.report_deadline, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.report_deadline = TEN_BILLION;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndReceivedTime_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.received_time, null, null);
        resultsParser.characters(TEN_BILLION_STR.toCharArray(), 0, TEN_BILLION_STR.length());
        resultsParser.endElement(null, Result.Fields.received_time, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.received_time = TEN_BILLION;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndEstimatedCpuTimeRemaining_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.estimated_cpu_time_remaining, null, null);
        resultsParser.characters(TEN_BILLION_STR.toCharArray(), 0, TEN_BILLION_STR.length());
        resultsParser.endElement(null, Result.Fields.estimated_cpu_time_remaining, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.estimated_cpu_time_remaining = TEN_BILLION;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndExitStatus_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.exit_status, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.exit_status, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.exit_status = 0;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndSuspendedViaGuiIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.suspended_via_gui, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.suspended_via_gui, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.suspended_via_gui = false;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndSuspendedViaGuiIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.suspended_via_gui, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.suspended_via_gui, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.suspended_via_gui = true;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndProjectSuspendedViaGuiIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.project_suspended_via_gui, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.project_suspended_via_gui, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.project_suspended_via_gui = false;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndProjectSuspendedViaGuiIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.project_suspended_via_gui, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.project_suspended_via_gui, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.project_suspended_via_gui = true;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForNonActiveTaskHasNameAndResources_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.resources, null, null);
        resultsParser.characters("Resources".toCharArray(), 0, "Resources".length());
        resultsParser.endElement(null, Result.Fields.resources, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.resources = "Resources";

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndActiveTaskState_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.active_task_state, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.active_task_state, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.active_task_state = 1;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndAppVersionNumber_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.app_version_num, null, null);
        resultsParser.characters("10".toCharArray(), 0, 2);
        resultsParser.endElement(null, Result.Fields.app_version_num, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.app_version_num = 10;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndSchedulerState_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.scheduler_state, null, null);
        resultsParser.characters("1000".toCharArray(), 0, "1000".length());
        resultsParser.endElement(null, Result.Fields.scheduler_state, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.scheduler_state = 1000;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndCheckpointCpuTime_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.checkpoint_cpu_time, null, null);
        resultsParser.characters("150000".toCharArray(), 0, "150000".length());
        resultsParser.endElement(null, Result.Fields.checkpoint_cpu_time, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.checkpoint_cpu_time = 150_000;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndCurrentCpuTime_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.current_cpu_time, null, null);
        resultsParser.characters("100000".toCharArray(), 0, "100000".length());
        resultsParser.endElement(null, Result.Fields.current_cpu_time, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.current_cpu_time = 100_000;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndFractionDone_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.fraction_done, null, null);
        resultsParser.characters("0.67".toCharArray(), 0, 4);
        resultsParser.endElement(null, Result.Fields.fraction_done, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.fraction_done = 0.67f;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndElapsedTime_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.elapsed_time, null, null);
        resultsParser.characters("2500".toCharArray(), 0, 4);
        resultsParser.endElement(null, Result.Fields.elapsed_time, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.elapsed_time = 2500;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndSwapSize_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.swap_size, null, null);
        resultsParser.characters("2500".toCharArray(), 0, 4);
        resultsParser.endElement(null, Result.Fields.swap_size, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.swap_size = 2500;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndWorkingSetSizeSmoothed_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.working_set_size_smoothed, null, null);
        resultsParser.characters("2500".toCharArray(), 0, 4);
        resultsParser.endElement(null, Result.Fields.working_set_size_smoothed, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.working_set_size_smoothed = 2500;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndEstimatedCpuTimeRemaining_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.estimated_cpu_time_remaining, null, null);
        resultsParser.characters("1000".toCharArray(), 0, 4);
        resultsParser.endElement(null, Result.Fields.estimated_cpu_time_remaining, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.estimated_cpu_time_remaining = 1000;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndSupportsGraphicsIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.supports_graphics, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.supports_graphics, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.supports_graphics = false;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndSupportsGraphicsIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.supports_graphics, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.supports_graphics, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.supports_graphics = true;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndGraphicsModeAcked_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.graphics_mode_acked, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.graphics_mode_acked, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.graphics_mode_acked = 1;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndTooLargeIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.too_large, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.too_large, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.too_large = false;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndTooLargeIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.too_large, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.too_large, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.too_large = true;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndNeedsShMemIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.needs_shmem, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.needs_shmem, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.needs_shmem = false;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndNeedsShmemIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.needs_shmem, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.needs_shmem, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.needs_shmem = true;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndEdfScheduledIs0_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.edf_scheduled, null, null);
        resultsParser.characters("0".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.edf_scheduled, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.edf_scheduled = false;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndEdfScheduledIs1_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.edf_scheduled, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.edf_scheduled, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.edf_scheduled = true;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndPid_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.pid, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.pid, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.pid = 1;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndSlot_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.slot, null, null);
        resultsParser.characters("1".toCharArray(), 0, 1);
        resultsParser.endElement(null, Result.Fields.slot, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.slot = 1;

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndGraphicsExecPath_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.graphics_exec_path, null, null);
        resultsParser.characters("/path/to/graphics".toCharArray(), 0, "/path/to/graphics".length());
        resultsParser.endElement(null, Result.Fields.graphics_exec_path, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.graphics_exec_path = "/path/to/graphics";

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }

    @Test
    public void testParser_whenXmlResultForActiveTaskHasNameAndSlotPath_thenExpectListWithMatchingResult()
            throws SAXException {
        resultsParser.startElement(null, ResultsParser.RESULT_TAG, null, null);
        resultsParser.startElement(null, RPCCommonTags.NAME, null, null);
        resultsParser.characters(NAME.toCharArray(), 0, NAME.length());
        resultsParser.endElement(null, RPCCommonTags.NAME, null);
        resultsParser.startElement(null, Result.Fields.active_task, null, null);
        resultsParser.startElement(null, Result.Fields.slot_path, null, null);
        resultsParser.characters("/path/to/slot".toCharArray(), 0, "/path/to/slot".length());
        resultsParser.endElement(null, Result.Fields.slot_path, null);
        resultsParser.endElement(null, Result.Fields.active_task, null);
        resultsParser.endElement(null, ResultsParser.RESULT_TAG, null);

        expected.name = NAME;
        expected.active_task = true;
        expected.slot_path = "/path/to/slot";

        assertEquals(Collections.singletonList(expected), resultsParser.getResults());
    }
}