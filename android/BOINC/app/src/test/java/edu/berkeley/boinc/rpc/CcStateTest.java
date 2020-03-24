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

import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

public class CcStateTest {
    private static final String URL_1 = "URL 1";
    private static final String URL_2 = "URL 2";

    private CcState ccState;

    private App app;
    private App app1;

    private Project project;
    private Project project1;

    @Before
    public void setUp() {
        ccState = new CcState();
        project = new Project();
        project1 = new Project();
        app = new App();
        app1 = new App();

        AppVersion appVersion = new AppVersion();
        Workunit workunit = new Workunit();

        project.master_url = URL_1;
        project1.master_url = URL_1;

        app.name = "App";
        app1.name = "App";
        app.project = project;
        app1.project = project1;

        workunit.name = "Work Unit";
        workunit.project = project;

        appVersion.plan_class = "Plan Class";
        appVersion.app = app;
        appVersion.project = project;
        appVersion.version_num = 1;

        ccState.apps.add(app);
        ccState.workunits.add(workunit);
        ccState.app_versions.add(appVersion);
    }

    @Test
    public void testLookupApp_whenAppListIsEmpty_thenExpectNull() {
        ccState.clearArrays();

        assertNull(ccState.lookup_app(null, null));
    }

    @Test
    public void testLookupApp_whenAppListHasOneAppAndParametersAreNull_thenExpectNull() {
        assertNull(ccState.lookup_app(null, null));
    }

    @Test
    public void testLookupApp_whenAppListHasOneAppAndProjectMatchesAppProjectAndAppNameIsNull_thenExpectNull() {
        assertNull(ccState.lookup_app(project, null));
    }

    @Test
    public void testLookupApp_whenAppListHasOneAppAndProjectDoesntMatch_thenExpectNull() {
        Project project1 = new Project();
        project1.master_url = URL_2;

        assertNull(ccState.lookup_app(project1, "App"));
    }

    @Test
    public void testLookupApp_whenAppListHasOneAppAndAppNameDoesntMatch_thenExpectNull() {
        Project project1 = new Project();
        project1.master_url = URL_1;

        assertNull(ccState.lookup_app(project1, "App 2"));
    }

    @Test
    public void testLookupApp_whenAppListHasOneAppAndProjectMatchesAppProjectAndNameMatchesAppName_thenExpectApp() {
        final App appFound = ccState.lookup_app(project, "App");

        assertNotNull(appFound);
        assertEquals("App", appFound.name);
        assertEquals(project, appFound.project);
    }

    @Test
    public void testLookupWorkUnit_whenWorkUnitListIsEmpty_thenExpectNull() {
        ccState.clearArrays();

        assertNull(ccState.lookup_wu(null, null));
    }

    @Test
    public void testLookupWorkUnit_whenWorkUnitListHasOneWorkUnitAndParametersAreNull_thenExpectNull() {
        assertNull(ccState.lookup_wu(null, null));
    }

    @Test
    public void testLookupWorkUnit_whenWorkUnitListHasOneWorkUnitAndProjectDoesntMatch_thenExpectNull() {
        project1.master_url = URL_2;

        assertNull(ccState.lookup_wu(project1, "Work Unit"));
    }

    @Test
    public void testLookupWorkUnit_whenWorkUnitListHasOneWorkUnitAndWorkUnitNameDoesntMatch_thenExpectNull() {
        assertNull(ccState.lookup_wu(project1, "Work Unit 2"));
    }

    @Test
    public void testLookupWorkUnit_whenWorkUnitListHasOneWorkUnitAndParametersMatch_thenExpectWorkUnit() {
        final Workunit workUnitFound = ccState.lookup_wu(project1, "Work Unit");

        assertNotNull(workUnitFound);
        assertEquals(project, workUnitFound.project);
        assertEquals("Work Unit", workUnitFound.name);
    }

    @Test
    public void testLookupAppVersion_whenAppVersionListIsEmpty_thenExpectNull() {
        ccState.clearArrays();

        assertNull(ccState.lookup_app_version(null, null, 0, null));
    }

    @Test
    public void testLookupAppVersion_whenAppVersionListHasOneAppVersionAndParametersAreNull_thenExpectNull() {
        assertNull(ccState.lookup_app_version(null, null, 0, null));
    }

    @Test
    public void testLookupAppVersion_whenAppVersionListHasOneAppVersionAndProjectDoesntMatch_thenExpectNull() {
        project1.master_url = URL_2;

        assertNull(ccState.lookup_app_version(project1, app1, 1, "Plan Class"));
    }

    @Test
    public void testLookupAppVersion_whenAppVersionListHasOneAppVersionAndAppDoesntMatch_thenExpectNull() {
        app1.name = "App 2";

        assertNull(ccState.lookup_app_version(project1, app1, 1, "Plan Class"));
    }

    @Test
    public void testLookupAppVersion_whenAppVersionListHasOneAppVersionAndVersionNumDoesntMatch_thenExpectNull() {
        assertNull(ccState.lookup_app_version(project1, app1, 0, "Plan Class"));
    }

    @Test
    public void testLookupAppVersion_whenAppVersionListHasOneAppVersionAndPlanClassDoesntMatch_thenExpectNull() {
        assertNull(ccState.lookup_app_version(project1, app1, 0, "Plan Class 2"));
    }

    @Test
    public void testLookupAppVersion_whenAppVersionListHasOneAppVersionAndParametersMatch_thenExpectAppVersion() {
        final AppVersion foundAppVersion =
                ccState.lookup_app_version(project1, app1, 1, "Plan Class");

        assertNotNull(foundAppVersion);
        assertEquals(project, foundAppVersion.project);
        assertEquals(app, foundAppVersion.app);
        assertEquals("Plan Class", foundAppVersion.plan_class);
    }
}