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
public class ProjectInfoParserTest {
    private static final String PROJECT_INFO_NAME = "Project Info";

    private ProjectInfoParser projectInfoParser;
    private ProjectInfo expected;

    @Before
    public void setUp() {
        projectInfoParser = new ProjectInfoParser();
        expected = new ProjectInfo();
    }

    @Test
    public void testParse_whenRpcStringIsNull_thenExpectNullPointerException() {
        mockStatic(Xml.class);

        assertThrows(NullPointerException.class, () -> ProjectInfoParser.parse(null));
    }

    @Test
    public void testParse_whenRpcStringIsEmpty_thenExpectEmptyList() {
        mockStatic(Xml.class);

        assertTrue(ProjectInfoParser.parse("").isEmpty());
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectElementStarted() throws SAXException {
        projectInfoParser.startElement(null, "", null, null);

        assertTrue(projectInfoParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsProjectTag_thenExpectElementNotStarted() throws SAXException {
        projectInfoParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectInfoParser.endElement(null, RPCCommonTags.PROJECT, null);

        assertFalse(projectInfoParser.mElementStarted);
    }

    @Test
    public void testParser_whenLocalNameIsEmpty_thenExpectEmptyList() throws SAXException {
        projectInfoParser.startElement(null, "", null, null);

        assertTrue(projectInfoParser.getProjectInfos().isEmpty());
    }

    @Test
    public void testParser_whenXmlProjectInfoHasNoElements_thenExpectEmptyList()
            throws SAXException {
        projectInfoParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectInfoParser.endElement(null, RPCCommonTags.PROJECT, null);

        assertTrue(projectInfoParser.getProjectInfos().isEmpty());
    }

    @Test
    public void testParser_whenXmlProjectInfoHasName_thenExpectListWithMatchingProjectInfo()
            throws SAXException {
        projectInfoParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectInfoParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length());
        projectInfoParser.endElement(null, RPCCommonTags.NAME, null);
        projectInfoParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setName(PROJECT_INFO_NAME);

        assertEquals(Collections.singletonList(expected), projectInfoParser.getProjectInfos());
    }

    @Test
    public void testParser_whenXmlProjectInfoHasNameAndUrl_thenExpectListWithMatchingProjectInfo()
            throws SAXException {
        projectInfoParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectInfoParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length());
        projectInfoParser.endElement(null, RPCCommonTags.NAME, null);
        projectInfoParser.startElement(null, RPCCommonTags.URL, null, null);
        projectInfoParser.characters("Project URL".toCharArray(), 0, "Project URL".length());
        projectInfoParser.endElement(null, RPCCommonTags.URL, null);
        projectInfoParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setName(PROJECT_INFO_NAME);
        expected.setUrl("Project URL");

        assertEquals(Collections.singletonList(expected), projectInfoParser.getProjectInfos());
    }

    @Test
    public void testParser_whenXmlProjectInfoHasNameAndGeneralArea_thenExpectListWithMatchingProjectInfo()
            throws SAXException {
        projectInfoParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectInfoParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length());
        projectInfoParser.endElement(null, RPCCommonTags.NAME, null);
        projectInfoParser.startElement(null, ProjectInfo.Fields.GENERAL_AREA, null,
                                       null);
        projectInfoParser.characters("Project General Area".toCharArray(), 0,
                                     "Project General Area".length());
        projectInfoParser.endElement(null, ProjectInfo.Fields.GENERAL_AREA, null);
        projectInfoParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setName(PROJECT_INFO_NAME);
        expected.setGeneralArea("Project General Area");

        assertEquals(Collections.singletonList(expected), projectInfoParser.getProjectInfos());
    }

    @Test
    public void testParser_whenXmlProjectInfoHasNameAndSpecificArea_thenExpectListWithMatchingProjectInfo()
            throws SAXException {
        projectInfoParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectInfoParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length());
        projectInfoParser.endElement(null, RPCCommonTags.NAME, null);
        projectInfoParser.startElement(null, ProjectInfo.Fields.SPECIFIC_AREA, null,
                                       null);
        projectInfoParser.characters("Project Specific Area".toCharArray(), 0,
                                     "Project Specific Area".length());
        projectInfoParser.endElement(null, ProjectInfo.Fields.SPECIFIC_AREA, null);
        projectInfoParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setName(PROJECT_INFO_NAME);
        expected.setSpecificArea("Project Specific Area");

        assertEquals(Collections.singletonList(expected), projectInfoParser.getProjectInfos());
    }

    @Test
    public void testParser_whenXmlProjectInfoHasNameAndDescription_thenExpectListWithMatchingProjectInfo()
            throws SAXException {
        projectInfoParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectInfoParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length());
        projectInfoParser.endElement(null, RPCCommonTags.NAME, null);
        projectInfoParser.startElement(null, RPCCommonTags.DESCRIPTION, null,
                                       null);
        projectInfoParser.characters("Project Description".toCharArray(), 0,
                                     "Project Description".length());
        projectInfoParser.endElement(null, RPCCommonTags.DESCRIPTION, null);
        projectInfoParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setName(PROJECT_INFO_NAME);
        expected.setDescription("Project Description");

        assertEquals(Collections.singletonList(expected), projectInfoParser.getProjectInfos());
    }

    @Test
    public void testParser_whenXmlProjectInfoHasNameAndHome_thenExpectListWithMatchingProjectInfo()
            throws SAXException {
        projectInfoParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectInfoParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length());
        projectInfoParser.endElement(null, RPCCommonTags.NAME, null);
        projectInfoParser.startElement(null, ProjectInfo.Fields.HOME, null,
                                       null);
        projectInfoParser.characters("Project Home".toCharArray(), 0, "Project Home".length());
        projectInfoParser.endElement(null, ProjectInfo.Fields.HOME, null);
        projectInfoParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setName(PROJECT_INFO_NAME);
        expected.setHome("Project Home");

        assertEquals(Collections.singletonList(expected), projectInfoParser.getProjectInfos());
    }

    @Test
    public void testParser_whenXmlProjectInfoHasNameAndImageUrl_thenExpectListWithMatchingProjectInfo()
            throws SAXException {
        projectInfoParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectInfoParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length());
        projectInfoParser.endElement(null, RPCCommonTags.NAME, null);
        projectInfoParser.startElement(null, ProjectInfo.Fields.IMAGE_URL, null,
                                       null);
        projectInfoParser.characters("Project Image URL".toCharArray(), 0,
                                     "Project Image URL".length());
        projectInfoParser.endElement(null, ProjectInfo.Fields.IMAGE_URL, null);
        projectInfoParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setName(PROJECT_INFO_NAME);
        expected.setImageUrl("Project Image URL");

        assertEquals(Collections.singletonList(expected), projectInfoParser.getProjectInfos());
    }

    @Test
    public void testParser_whenXmlProjectInfoHasNameAndSummary_thenExpectListWithMatchingProjectInfo()
            throws SAXException {
        projectInfoParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectInfoParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length());
        projectInfoParser.endElement(null, RPCCommonTags.NAME, null);
        projectInfoParser.startElement(null, ProjectInfo.Fields.SUMMARY, null,
                                       null);
        projectInfoParser.characters("Project Summary".toCharArray(), 0,
                                     "Project Summary".length());
        projectInfoParser.endElement(null, ProjectInfo.Fields.SUMMARY, null);
        projectInfoParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setName(PROJECT_INFO_NAME);
        expected.setSummary("Project Summary");

        assertEquals(Collections.singletonList(expected), projectInfoParser.getProjectInfos());
    }

    @Test
    public void testParser_whenXmlProjectInfoHasNameAndOnePlatform_thenExpectListWithMatchingProjectInfo()
            throws SAXException {
        projectInfoParser.startElement(null, RPCCommonTags.PROJECT, null, null);
        projectInfoParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectInfoParser.characters(PROJECT_INFO_NAME.toCharArray(), 0, PROJECT_INFO_NAME.length());
        projectInfoParser.endElement(null, RPCCommonTags.NAME, null);
        projectInfoParser.startElement(null, ProjectInfo.Fields.PLATFORMS, null, null);
        projectInfoParser.startElement(null, RPCCommonTags.NAME, null, null);
        projectInfoParser.characters("Platform Name".toCharArray(), 0, "Platform Name".length());
        projectInfoParser.endElement(null, RPCCommonTags.NAME, null);
        projectInfoParser.endElement(null, ProjectInfo.Fields.PLATFORMS, null);
        projectInfoParser.endElement(null, RPCCommonTags.PROJECT, null);

        expected.setName(PROJECT_INFO_NAME);
        expected.setPlatforms(Collections.singletonList("Platform Name"));

        assertEquals(Collections.singletonList(expected), projectInfoParser.getProjectInfos());
    }
}
