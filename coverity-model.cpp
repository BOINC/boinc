// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

/*
 * Coverity Scan Modeling file for BOINC
 *
 * This defines behaviour of functions that Coverity Scan is not aware of.
 *
 * For how to create a model see:
 * https://scan.coverity.com/tune and https://scan.coverity.com/models#overriding
 *
 * If you add anything here it has no immediate effect. A user with the
 * Maintainer/Owner role on scan.coverity.com has to upload the file to
 * https://scan.coverity.com/projects/boinc-boinc?tab=analysis_settings
 *
 **/

// the dir string is kind of sanitized here
// prevents tainted string defects involving SCHED_CONFIG::project_path()
//
bool is_project_dir(const char *dir) {
    bool ok_string;
    if (ok_string == true) {
        __coverity_tainted_string_sanitize_content__((void*)dir);
        return true;
    }
    return false;
}
