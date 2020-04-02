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

data class CcState
@JvmOverloads // generates overloaded constructors
constructor(
        var versionInfo: VersionInfo? = null,
        var hostInfo: HostInfo? = null,
        var haveAti: Boolean = false,
        var haveCuda: Boolean = false,
        val projects: MutableList<Project> = mutableListOf(),
        val apps: MutableList<App> = mutableListOf(),
        val appVersions: MutableList<AppVersion> = mutableListOf(),
        val workUnits: MutableList<WorkUnit> = mutableListOf(),
        val results: MutableList<Result> = mutableListOf()
) {
    fun clearArrays() {
        projects.clear()
        apps.clear()
        appVersions.clear()
        workUnits.clear()
        results.clear()
    }

    fun lookupApp(project: Project?, appName: String?): App? {
        return apps.filter { it.project == project }
                .firstOrNull { it.name.equals(appName, ignoreCase = true) }
    }

    fun lookupWorkUnit(project: Project?, workUnitName: String?): WorkUnit? {
        return workUnits.filter { it.project == project }
                .firstOrNull { it.name.equals(workUnitName, ignoreCase = true) }
    }

    fun lookupAppVersion(project: Project?, app: App?, versionNum: Int, planClass: String?): AppVersion? {
        // Sequences process elements lazily, which can improve performance with large collections and
        // complex operations.
        return appVersions.asSequence()
                .filter { it.project == project } //Check if projects match
                .filter { it.app == app } //Check if app matches
                .filter { it.versionNum == versionNum } //Check version_num
                .firstOrNull { it.planClass.equals(planClass, ignoreCase = true) } //Check plan class
    }

    object Fields {
        const val HAVE_ATI = "have_ati"
        const val HAVE_CUDA = "have_cuda"
    }
}
