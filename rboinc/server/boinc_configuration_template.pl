# Site-specific configuration constants (used by the server side
# component). 
#
# $Id: boinc_configuration_template.pl 356 2010-03-02 15:00:31Z toni $


# This file is part of RemoteBOINC.
# Copyright (C) 2010 Universitat Pompeu Fabra

# RemoteBOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.

# RemoteBOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.


my $cgi="";

return {
# Dav incoming directory, as seen by this script (path on server's
# filesystem)
    DAV_DIR  =>  "",

# Boinc's project dir, used for sumbission
    PROJECT_DIR   =>  "",

# Workflow_results directory, where the input and assimilator's output
# files will go, and scripts will be launched.
    WORKFLOW_DIR  =>  "",

# DAV url as seen by the clients
    DAV_URL  =>  "",

# Encodeinput parameters. Architecture will be appended at the end 
# of executable name (x86_64 or i686).
    ENCODE_EXECUTABLE => "$cgi/etc/encodeinput",
    ENCODE_CODE => xxxxxxx,

# Directory for miscellaneous files (eg. defaults)
    ETC_DIR => "$cgi/etc",

# Minimum age for files to be removed during remote retrieval, in days
# (possibly fractional). Should be at least the frequency with which
# validator is run (?)
    REMOVE_MIN_AGE => 1./24,

# Defaults if not specified - not under user control
    DEFAULT_WU_TEMPLATE => 'std_md_wu',
    DEFAULT_RESULT_TEMPLATE => 'std_md_result',
    DEFAULT_EMAIL => 'nobody@nil.es',
    DEFAULT_DELAY_BOUND => 5*24*3600,

# Defaults if not specified - under user control
    DEFAULT_APP_NAME => 'acemd',
    DEFAULT_NUMSTEPS => 1,
    DEFAULT_PRIORITY => 1,
    DEFAULT_TARGET_NRESULTS => 1,
    DEFAULT_MIN_QUORUM => 1,


};

