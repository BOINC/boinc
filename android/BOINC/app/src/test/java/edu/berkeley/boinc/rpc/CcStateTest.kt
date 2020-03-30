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
package edu.berkeley.boinc.rpc

import com.google.common.testing.EqualsTester
import org.junit.Before
import org.junit.Test
import kotlin.test.junit.JUnitAsserter

class CcStateTest {
    private lateinit var ccState: CcState
    private lateinit var app: App
    private lateinit var app1: App
    private lateinit var project: Project
    private lateinit var project1: Project

    @Before
    fun setUp() {
        ccState = CcState()
        project = Project()
        project1 = Project()
        app = App()
        app1 = App()

        val appVersion = AppVersion()
        val workUnit = WorkUnit()

        project.masterURL = URL_1
        project1.masterURL = URL_1
        app.name = APP
        app1.name = APP
        app.project = project
        app1.project = project1

        workUnit.name = WORK_UNIT
        workUnit.project = project
        appVersion.planClass = PLAN_CLASS
        appVersion.app = app
        appVersion.project = project
        appVersion.versionNum = 1

        ccState.apps.add(app)
        ccState.workUnits.add(workUnit)
        ccState.appVersions.add(appVersion)
    }

    @Test
    fun `Test equals() and hashCode()`() {
        EqualsTester().addEqualityGroup(CcState(), CcState())
                .addEqualityGroup(CcState(VersionInfo()))
                .addEqualityGroup(CcState(hostInfo = HostInfo()))
                .addEqualityGroup(CcState(haveAti = true))
                .addEqualityGroup(CcState(haveCuda = true))
                .addEqualityGroup(ccState)
                .testEquals()
    }

    @Test
    fun `Expect lookupApp() to return null when app list is empty`() {
        ccState.clearArrays()
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupApp(null, null))
    }

    @Test
    fun `Expect lookupApp() to return null when app list has one app and parameters are null`() {
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupApp(null, null))
    }

    @Test
    fun `Expect lookupApp() to return null when app list has one app, project matches app project and app name is null`() {
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupApp(project, null))
    }

    @Test
    fun `Expect lookupApp() to return null when app list has one app and project doesn't match`() {
        val project1 = Project(masterURL = URL_2)

        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupApp(project1, APP))
    }

    @Test
    fun `Expect lookupApp() to return null when app list has one app and app name doesn't match`() {
        val project1 = Project(masterURL = URL_1)

        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupApp(project1, "$APP 2"))
    }

    @Test
    fun `Expect lookupApp() to return app when app list has one app and project and app name match`() {
        val appFound = ccState.lookupApp(project, APP)

        JUnitAsserter.assertEquals("Expected '$APP'", APP, appFound!!.name)
        JUnitAsserter.assertEquals(PROJECT_MSG, project, appFound.project)
    }

    @Test
    fun `Expect lookupWorkUnit() to return null when work unit list is empty`() {
        ccState.clearArrays()
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupWorkUnit(null, null))
    }

    @Test
    fun `Expect lookupWorkUnit() to return null when work unit list has one work unit and parameters are null`() {
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupWorkUnit(null, null))
    }

    @Test
    fun `Expect lookupWorkUnit() to return null when work unit list has one work unit and project doesn't match`() {
        project1.masterURL = URL_2
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupWorkUnit(project1, WORK_UNIT))
    }

    @Test
    fun `Expect lookupWorkUnit() to return null when work unit list has one work unit and work unit name doesn't match`() {
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupWorkUnit(project1, "$WORK_UNIT 2"))
    }

    @Test
    fun `Expect lookupWorkUnit() to return work unit when work unit list has one work unit and parameters match`() {
        val workUnitFound = ccState.lookupWorkUnit(project1, WORK_UNIT)
        JUnitAsserter.assertEquals(PROJECT_MSG, project, workUnitFound!!.project)
        JUnitAsserter.assertEquals("Expected work unit", WORK_UNIT, workUnitFound.name)
    }

    @Test
    fun `Expect lookupAppVersion() to return null when app version list is empty`() {
        ccState.clearArrays()
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupAppVersion(null, null, 0, null))
    }

    @Test
    fun `Expect lookupAppVersion() to return null when app version list has one app version and parameters are null`() {
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupAppVersion(null, null, 0, null))
    }

    @Test
    fun `Expect lookupAppVersion() to return null when app version list has one app version and project doesn't match`() {
        project1.masterURL = URL_2
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupAppVersion(project1, app1, 1, PLAN_CLASS))
    }

    @Test
    fun `Expect lookupAppVersion() to return null when app version list has one app version and app doesn't match`() {
        app1.name = "App 2"
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupAppVersion(project1, app1, 1, PLAN_CLASS))
    }

    @Test
    fun `Expect lookupAppVersion() to return null when app version list has one app version and version number doesn't match`() {
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupAppVersion(project1, app1, 0, PLAN_CLASS))
    }

    @Test
    fun `Expect lookupAppVersion() to return null when app version list has one app version and plan class doesn't match`() {
        JUnitAsserter.assertNull(NULL_MSG, ccState.lookupAppVersion(project1, app1, 0, "$PLAN_CLASS 2"))
    }

    @Test
    fun `Expect lookupAppVersion() to return app version when app version list has one app version and parameters match`() {
        val foundAppVersion = ccState.lookupAppVersion(project1, app1, 1, PLAN_CLASS)
        JUnitAsserter.assertEquals(PROJECT_MSG, project, foundAppVersion!!.project)
        JUnitAsserter.assertEquals("Expected app", app, foundAppVersion.app)
        JUnitAsserter.assertEquals("Expected plan class", PLAN_CLASS, foundAppVersion.planClass)
    }

    companion object {
        private const val APP = "App"
        private const val PLAN_CLASS = "Plan Class"
        private const val URL_1 = "URL 1"
        private const val URL_2 = "URL 2"
        private const val WORK_UNIT = "Work Unit"

        private const val NULL_MSG = "Expected null"
        private const val PROJECT_MSG = "Expected project"
    }
}
