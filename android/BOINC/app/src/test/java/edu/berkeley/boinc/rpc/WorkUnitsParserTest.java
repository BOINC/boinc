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
public class WorkUnitsParserTest {
    private static final String WORK_UNIT_NAME = "Work Unit";

    private WorkUnitsParser workUnitsParser;
    private WorkUnit expected;

    @Before
    public void setUp() {
        workUnitsParser = new WorkUnitsParser();
        expected = new WorkUnit();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectEmptyList() {
        mockStatic(Xml.class);

        assertEquals(Collections.emptyList(), WorkUnitsParser.parse(null));
    }

    @Test
    public void testParser_whenOnlyStartElementIsRun_thenExpectElementStarted() throws SAXException {
        workUnitsParser.startElement(null, "", null, null);

        assertTrue(workUnitsParser.mElementStarted);
    }

    @Test
    public void testParser_whenBothStartElementAndEndElementAreRun_thenExpectElementNotStarted()
            throws SAXException {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null);
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null);

        assertFalse(workUnitsParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectEmptyList() throws SAXException {
        workUnitsParser.startElement(null, "", null, null);

        assertTrue(workUnitsParser.getWorkUnits().isEmpty());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNoElements_thenExpectEmptyList()
            throws SAXException {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null);
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null);

        assertTrue(workUnitsParser.getWorkUnits().isEmpty());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasBlankName_thenExpectEmptyList()
            throws SAXException {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null);
        workUnitsParser.startElement(null, RPCCommonTags.NAME, null, null);
        workUnitsParser.characters("".toCharArray(), 0, 0);
        workUnitsParser.endElement(null, RPCCommonTags.NAME, null);
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null);

        assertTrue(workUnitsParser.getWorkUnits().isEmpty());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasOnlyName_thenExpectListWithMatchingWorkUnit()
            throws SAXException {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null);
        workUnitsParser.startElement(null, RPCCommonTags.NAME, null, null);
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workUnitsParser.endElement(null, RPCCommonTags.NAME, null);
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null);

        expected.setName(WORK_UNIT_NAME);

        assertEquals(Collections.singletonList(expected), workUnitsParser.getWorkUnits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasOnlyNameAndAppName_thenExpectListWithMatchingWorkUnit()
            throws SAXException {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null);
        workUnitsParser.startElement(null, RPCCommonTags.NAME, null, null);
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workUnitsParser.endElement(null, RPCCommonTags.NAME, null);
        workUnitsParser.startElement(null, WorkUnit.Fields.APP_NAME, null, null);
        workUnitsParser.characters("App".toCharArray(), 0, 3);
        workUnitsParser.endElement(null, WorkUnit.Fields.APP_NAME, null);
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null);

        expected.setName(WORK_UNIT_NAME);
        expected.setAppName("App");

        assertEquals(Collections.singletonList(expected), workUnitsParser.getWorkUnits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNameAndInvalidVersionNum_thenExpectListWithWorkUnitWithOnlyName()
            throws SAXException {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null);
        workUnitsParser.startElement(null, RPCCommonTags.NAME, null, null);
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workUnitsParser.endElement(null, RPCCommonTags.NAME, null);
        workUnitsParser.startElement(null, WorkUnit.Fields.VERSION_NUM, null, null);
        workUnitsParser.characters("One".toCharArray(), 0, 3);
        workUnitsParser.endElement(null, WorkUnit.Fields.VERSION_NUM, null);
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null);

        expected.setName(WORK_UNIT_NAME);

        assertEquals(Collections.singletonList(expected), workUnitsParser.getWorkUnits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNameAndVersionNum_thenExpectListWithWorkUnitWithOnlyName()
            throws SAXException {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null);
        workUnitsParser.startElement(null, RPCCommonTags.NAME, null, null);
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workUnitsParser.endElement(null, RPCCommonTags.NAME, null);
        workUnitsParser.startElement(null, WorkUnit.Fields.VERSION_NUM, null, null);
        workUnitsParser.characters("1".toCharArray(), 0, 1);
        workUnitsParser.endElement(null, WorkUnit.Fields.VERSION_NUM, null);
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null);

        expected.setName(WORK_UNIT_NAME);
        expected.setVersionNum(1);

        assertEquals(Collections.singletonList(expected), workUnitsParser.getWorkUnits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNameAndRscFloatingPointOpsEst_thenExpectListWithWorkUnitWithOnlyName()
            throws SAXException {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null);
        workUnitsParser.startElement(null, RPCCommonTags.NAME, null, null);
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workUnitsParser.endElement(null, RPCCommonTags.NAME, null);
        workUnitsParser.startElement(null, WorkUnit.Fields.RSC_FPOPS_EST, null, null);
        workUnitsParser.characters("1.5".toCharArray(), 0, 3);
        workUnitsParser.endElement(null, WorkUnit.Fields.RSC_FPOPS_EST, null);
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null);

        expected.setName(WORK_UNIT_NAME);
        expected.setRscFloatingPointOpsEst(1.5);

        assertEquals(Collections.singletonList(expected), workUnitsParser.getWorkUnits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNameAndRscFloatingPointOpsBound_thenExpectListWithWorkUnitWithOnlyName()
            throws SAXException {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null);
        workUnitsParser.startElement(null, RPCCommonTags.NAME, null, null);
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workUnitsParser.endElement(null, RPCCommonTags.NAME, null);
        workUnitsParser.startElement(null, WorkUnit.Fields.RSC_FPOPS_BOUND, null, null);
        workUnitsParser.characters("1.5".toCharArray(), 0, 3);
        workUnitsParser.endElement(null, WorkUnit.Fields.RSC_FPOPS_BOUND, null);
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null);

        expected.setName(WORK_UNIT_NAME);
        expected.setRscFloatingPointOpsBound(1.5);

        assertEquals(Collections.singletonList(expected), workUnitsParser.getWorkUnits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNameAndRscMemoryBound_thenExpectListWithWorkUnitWithOnlyName()
            throws SAXException {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null);
        workUnitsParser.startElement(null, RPCCommonTags.NAME, null, null);
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workUnitsParser.endElement(null, RPCCommonTags.NAME, null);
        workUnitsParser.startElement(null, WorkUnit.Fields.RSC_MEMORY_BOUND, null, null);
        workUnitsParser.characters("1.5".toCharArray(), 0, 3);
        workUnitsParser.endElement(null, WorkUnit.Fields.RSC_MEMORY_BOUND, null);
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null);

        expected.setName(WORK_UNIT_NAME);
        expected.setRscMemoryBound(1.5);

        assertEquals(Collections.singletonList(expected), workUnitsParser.getWorkUnits());
    }

    @Test
    public void testParser_whenXmlWorkUnitHasNameAndRscDiskBound_thenExpectListWithWorkUnitWithOnlyName()
            throws SAXException {
        workUnitsParser.startElement(null, WorkUnitsParser.WORKUNIT_TAG, null, null);
        workUnitsParser.startElement(null, RPCCommonTags.NAME, null, null);
        workUnitsParser.characters(WORK_UNIT_NAME.toCharArray(), 0, WORK_UNIT_NAME.length());
        workUnitsParser.endElement(null, RPCCommonTags.NAME, null);
        workUnitsParser.startElement(null, WorkUnit.Fields.RSC_DISK_BOUND, null, null);
        workUnitsParser.characters("1.5".toCharArray(), 0, 3);
        workUnitsParser.endElement(null, WorkUnit.Fields.RSC_DISK_BOUND, null);
        workUnitsParser.endElement(null, WorkUnitsParser.WORKUNIT_TAG, null);

        expected.setName(WORK_UNIT_NAME);
        expected.setRscDiskBound(1.5);

        assertEquals(Collections.singletonList(expected), workUnitsParser.getWorkUnits());
    }
}
