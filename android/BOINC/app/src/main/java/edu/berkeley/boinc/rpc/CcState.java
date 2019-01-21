/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2012 University of California
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

import java.util.ArrayList;

public class CcState {
    public VersionInfo version_info;
    public HostInfo host_info;
    public ArrayList<Project> projects = new ArrayList<>();
    public ArrayList<App> apps = new ArrayList<>();
    public ArrayList<AppVersion> app_versions = new ArrayList<>();
    public ArrayList<Workunit> workunits = new ArrayList<>();
    public ArrayList<Result> results = new ArrayList<>();
    public boolean have_ati;
    public boolean have_cuda;

    public void clearArrays() {
        projects.clear();
        apps.clear();
        app_versions.clear();
        workunits.clear();
        results.clear();
    }

    public Project lookup_project(String testUrl) {
        for(int i = 0; i < projects.size(); i++) {
            if(projects.get(i).master_url.equalsIgnoreCase(testUrl)) {
                return projects.get(i);
            }
        }
        return null;
    }

    public App lookup_app(Project project, String appname) {
        for(int i = 0; i < apps.size(); i++) {
            if(!apps.get(i).project.compare(project)) {
                continue;
            }
            if(apps.get(i).name.equalsIgnoreCase(appname)) {
                return apps.get(i);
            }
        }
        return null;
    }

    public Workunit lookup_wu(Project project, String wu_name) {
        for(int i = 0; i < workunits.size(); i++) {
            if(!workunits.get(i).project.compare(project)) {
                //if(Logging.DEBUG) Log.d("Workunit", "Projects Do not compare");
                continue;
            }
            if(workunits.get(i).name.equalsIgnoreCase(wu_name)) {
                return workunits.get(i);
            }
        }
        return null;
    }

    public AppVersion lookup_app_version(Project project, App app, int version_num, String plan_class) {
        for(int i = 0; i < app_versions.size(); i++) {
            //Check if projects match...
            if(!app_versions.get(i).project.compare(project)) {
                continue;
            }
            //Check if app matches
            if(!app_versions.get(i).app.compare(app)) {
                continue;
            }
            //checks version_num
            if(app_versions.get(i).version_num != version_num) {
                continue;
            }
            //Checks plan class
            if(!app_versions.get(i).plan_class.equalsIgnoreCase(plan_class)) {
                continue;
            }
            return app_versions.get(i);
        }
        return null;
    }
}
