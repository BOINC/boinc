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

import static org.junit.Assert.*;
import static org.powermock.api.mockito.PowerMockito.mockStatic;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Xml.class)
public class WorkunitsParserTest {
    private static final String WORK_UNIT_NAME = "Work Unit";

    private WorkunitsParser workunitsParser;

    @Before
    public void setUp() {
        workunitsParser = new WorkunitsParser();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectEmptyList() {
        mockStatic(Xml.class);

        assertEquals(Collections.emptyList(), WorkunitsParser.parse(null));
    }

    @Test
    public void testParser_whenOnlyStartElementIsRun_thenExpectElementStarted() throws SAXException {
        workunitsParser.startElement(null, "", null, null);

        assertTrue(workunitsParser.mElementStarted);
    }

    @Test
    public void testParser_whenBothStartElementAndEndElementAreRun_thenExpectElementNotStarted()
            throws SAXException {
        workunitsParser.startElement(null, WorkunitsParser.WORKUNIT_TAG, null, null);
        workunitsParser.endElement(null, WorkunitsParser.WORKUNIT_TAG, null);

        assertFalse(workunitsParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectEmptyList() throws SAXException {
        workunitsParser.startElement(null, "", null, null);

        assertTrue(workunitsParser.getWorkunits().isEmpty());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNoElements_thenExpectEmptyList()
            throws SAXException {
        workunitsParser.startElement(null, WorkunitsParser.WORKUNIT_TAG, null, null);
        workunitsParser.endElement(null, WorkunitsParser.WORKUNIT_TAG, null);

        assertTrue(workunitsParser.getWorkunits().isEmpty());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasBlankName_thenExpectEmptyList()
            throws SAXException {
        workunitsParser.startElement(null, WorkunitsParser.WORKUNIT_TAG, null, null);
        workunitsParser.startElement(null, Workunit.Fields.name, null, null);
        workunitsParser.characters("".toCharArray(), 0, 0);
        workunitsParser.endElement(null, Workunit.Fields.name, null);
        workunitsParser.endElement(null, WorkunitsParser.WORKUNIT_TAG, null);

        assertTrue(workunitsParser.getWorkunits().isEmpty());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasOnlyName_thenExpectListWithMatchingWorkUnit()
            throws SAXException {
        workunitsParser.startElement(null, WorkunitsParser.WORKUNIT_TAG, null, null);
        workunitsParser.startElement(null, Workunit.Fields.name, null, null);
        workunitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workunitsParser.endElement(null, Workunit.Fields.name, null);
        workunitsParser.endElement(null, WorkunitsParser.WORKUNIT_TAG, null);

        final Workunit workunit = new Workunit();
        workunit.name = "Work Unit";

        assertEquals(Collections.singletonList(workunit), workunitsParser.getWorkunits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasOnlyNameAndAppName_thenExpectListWithMatchingWorkUnit()
            throws SAXException {
        workunitsParser.startElement(null, WorkunitsParser.WORKUNIT_TAG, null, null);
        workunitsParser.startElement(null, Workunit.Fields.name, null, null);
        workunitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workunitsParser.endElement(null, Workunit.Fields.name, null);
        workunitsParser.startElement(null, Workunit.Fields.app_name, null, null);
        workunitsParser.characters("App".toCharArray(), 0, 3);
        workunitsParser.endElement(null, Workunit.Fields.app_name, null);
        workunitsParser.endElement(null, WorkunitsParser.WORKUNIT_TAG, null);

        final Workunit workunit = new Workunit();
        workunit.name = WORK_UNIT_NAME;
        workunit.app_name = "App";

        assertEquals(Collections.singletonList(workunit), workunitsParser.getWorkunits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNameAndInvalidVersionNum_thenExpectListWithWorkUnitWithOnlyName()
            throws SAXException {
        workunitsParser.startElement(null, WorkunitsParser.WORKUNIT_TAG, null, null);
        workunitsParser.startElement(null, Workunit.Fields.name, null, null);
        workunitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workunitsParser.endElement(null, Workunit.Fields.name, null);
        workunitsParser.startElement(null, Workunit.Fields.version_num, null, null);
        workunitsParser.characters("One".toCharArray(), 0, 3);
        workunitsParser.endElement(null, Workunit.Fields.version_num, null);
        workunitsParser.endElement(null, WorkunitsParser.WORKUNIT_TAG, null);

        final Workunit workunit = new Workunit();
        workunit.name = WORK_UNIT_NAME;

        assertEquals(Collections.singletonList(workunit), workunitsParser.getWorkunits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNameAndVersionNum_thenExpectListWithWorkUnitWithOnlyName()
            throws SAXException {
        workunitsParser.startElement(null, WorkunitsParser.WORKUNIT_TAG, null, null);
        workunitsParser.startElement(null, Workunit.Fields.name, null, null);
        workunitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workunitsParser.endElement(null, Workunit.Fields.name, null);
        workunitsParser.startElement(null, Workunit.Fields.version_num, null, null);
        workunitsParser.characters("1".toCharArray(), 0, 1);
        workunitsParser.endElement(null, Workunit.Fields.version_num, null);
        workunitsParser.endElement(null, WorkunitsParser.WORKUNIT_TAG, null);

        final Workunit workunit = new Workunit();
        workunit.name = WORK_UNIT_NAME;
        workunit.version_num = 1;

        assertEquals(Collections.singletonList(workunit), workunitsParser.getWorkunits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNameAndRscFloatingPointOpsEst_thenExpectListWithWorkUnitWithOnlyName()
            throws SAXException {
        workunitsParser.startElement(null, WorkunitsParser.WORKUNIT_TAG, null, null);
        workunitsParser.startElement(null, Workunit.Fields.name, null, null);
        workunitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workunitsParser.endElement(null, Workunit.Fields.name, null);
        workunitsParser.startElement(null, Workunit.Fields.rsc_fpops_est, null, null);
        workunitsParser.characters("1.5".toCharArray(), 0, 3);
        workunitsParser.endElement(null, Workunit.Fields.rsc_fpops_est, null);
        workunitsParser.endElement(null, WorkunitsParser.WORKUNIT_TAG, null);

        final Workunit workunit = new Workunit();
        workunit.name = WORK_UNIT_NAME;
        workunit.rsc_fpops_est = 1.5;

        assertEquals(Collections.singletonList(workunit), workunitsParser.getWorkunits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNameAndRscFloatingPointOpsBound_thenExpectListWithWorkUnitWithOnlyName()
            throws SAXException {
        workunitsParser.startElement(null, WorkunitsParser.WORKUNIT_TAG, null, null);
        workunitsParser.startElement(null, Workunit.Fields.name, null, null);
        workunitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workunitsParser.endElement(null, Workunit.Fields.name, null);
        workunitsParser.startElement(null, Workunit.Fields.rsc_fpops_bound, null, null);
        workunitsParser.characters("1.5".toCharArray(), 0, 3);
        workunitsParser.endElement(null, Workunit.Fields.rsc_fpops_bound, null);
        workunitsParser.endElement(null, WorkunitsParser.WORKUNIT_TAG, null);

        final Workunit workunit = new Workunit();
        workunit.name = WORK_UNIT_NAME;
        workunit.rsc_fpops_bound = 1.5;

        assertEquals(Collections.singletonList(workunit), workunitsParser.getWorkunits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNameAndRscMemoryBound_thenExpectListWithWorkUnitWithOnlyName()
            throws SAXException {
        workunitsParser.startElement(null, WorkunitsParser.WORKUNIT_TAG, null, null);
        workunitsParser.startElement(null, Workunit.Fields.name, null, null);
        workunitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workunitsParser.endElement(null, Workunit.Fields.name, null);
        workunitsParser.startElement(null, Workunit.Fields.rsc_memory_bound, null, null);
        workunitsParser.characters("1.5".toCharArray(), 0, 3);
        workunitsParser.endElement(null, Workunit.Fields.rsc_memory_bound, null);
        workunitsParser.endElement(null, WorkunitsParser.WORKUNIT_TAG, null);

        final Workunit workunit = new Workunit();
        workunit.name = WORK_UNIT_NAME;
        workunit.rsc_memory_bound = 1.5;

        assertEquals(Collections.singletonList(workunit), workunitsParser.getWorkunits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNameAndRscDiskBound_thenExpectListWithWorkUnitWithOnlyName()
            throws SAXException {
        workunitsParser.startElement(null, WorkunitsParser.WORKUNIT_TAG, null, null);
        workunitsParser.startElement(null, Workunit.Fields.name, null, null);
        workunitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workunitsParser.endElement(null, Workunit.Fields.name, null);
        workunitsParser.startElement(null, Workunit.Fields.rsc_disk_bound, null, null);
        workunitsParser.characters("1.5".toCharArray(), 0, 3);
        workunitsParser.endElement(null, Workunit.Fields.rsc_disk_bound, null);
        workunitsParser.endElement(null, WorkunitsParser.WORKUNIT_TAG, null);

        final Workunit workunit = new Workunit();
        workunit.name = WORK_UNIT_NAME;
        workunit.rsc_disk_bound = 1.5;

        assertEquals(Collections.singletonList(workunit), workunitsParser.getWorkunits());
    }
}