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

import java.util.ArrayList

data class CcState(
        @JvmField
        var version_info: VersionInfo? = null,
        @JvmField
        var host_info: HostInfo? = null,
        @JvmField
        var have_ati: Boolean = false,
        @JvmField
        var have_cuda: Boolean = false,
        @JvmField
        val projects: MutableList<Project> = ArrayList(),
        @JvmField
        val apps: MutableList<App> = ArrayList(),
        @JvmField
        val app_versions: MutableList<AppVersion> = ArrayList(),
        @JvmField
        val workunits: MutableList<Workunit> = ArrayList(),
        @JvmField
        val results: MutableList<Result> = ArrayList()
) {
    fun clearArrays() {
        projects.clear()
        apps.clear()
        app_versions.clear()
        workunits.clear()
        results.clear()
    }

    fun lookup_app(project: Project?, appname: String?): App? {
        for (i in apps.indices) {
            if (apps[i].project != project) {
                continue
            }
            if (apps[i].name.equals(appname, ignoreCase = true)) {
                return apps[i]
            }
        }
        return null
    }

    fun lookup_wu(project: Project?, wu_name: String?): Workunit? {
        for (i in workunits.indices) {
            if (workunits[i].project != project) {
                continue
            }
            if (workunits[i].name.equals(wu_name, ignoreCase = true)) {
                return workunits[i]
            }
        }
        return null
    }

    fun lookup_app_version(project: Project?, app: App, version_num: Int, plan_class: String?): AppVersion? {
        for (i in app_versions.indices) {
            //Check if projects match...
            if (app_versions[i].project != project) {
                continue
            }
            //Check if app matches
            if (app_versions[i].app != app) {
                continue
            }
            //checks version_num
            if (app_versions[i].version_num != version_num) {
                continue
            }
            //Checks plan class
            if (!app_versions[i].plan_class.equals(plan_class, ignoreCase = true)) {
                continue
            }
            return app_versions[i]
        }
        return null
    }

    object Fields {
        const val HAVE_ATI = "have_ati"
        const val HAVE_CUDA = "have_cuda"
    }
}
