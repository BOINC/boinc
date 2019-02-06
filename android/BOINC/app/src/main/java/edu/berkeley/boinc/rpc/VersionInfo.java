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


public class VersionInfo {
    // all attributes are public for simple access
    public int major = 0;
    public int minor = 0;
    public int release = 0;
    public boolean prerelease = false;

    public boolean greater_than(VersionInfo vi) {
        if(major > vi.major) {
            return true;
        }
        else if(major < vi.major) {
            return false;
        }
        else if(minor > vi.minor) {
            return true;
        }
        else if(minor < vi.minor) {
            return false;
        }
        else {
            return (release > vi.release);
        }
    }

}
