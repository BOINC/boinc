# Site-specific configuration constants (used by the server side
# component). 
#
# $Id: boinc_configuration.pl 356 2010-03-02 15:00:31Z toni $



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



my $cgi="/home/boinc/toni/apache/cgi-bin";

return {
# Dav incoming directory, as seen by this script (path on server's
# filesystem)
    DAV_DIR  =>  "$cgi/../DAV",

# Boinc's project dir, used for sumbission
    PROJECT_DIR   =>  "/home/boinc/projects/LUNA",

# Workflow_results directory, where the input and assimilator's output
# files will go, and scripts will be launched.
    WORKFLOW_DIR  =>  "/home/boinc/projects/LUNA/workflow_results",

# DAV url as seen by the clients
    DAV_URL  =>  "http://XXXXX:8383/DAV/",

# Encodeinput parameters. Architecture will be appended at the end 
# of executable name (x86_64 or i686).
    ENCODE_EXECUTABLE => "$cgi/etc/encodeinput",
    ENCODE_CODE => -1,

# File extension table. The funny layout is to simplify XML-ing &
# lookups later.  The server will send this list to the client. The
# client will NOT download files if a file with the same name exists,
# OR one with the appended extensions.
    ALIAS_TABLE => [
	{ Extension => "_0",
	  Alias => [ ".log", ".log.gz", ".log.gz.bad" ] },
	{ Extension => "_1",
	  Alias => [ ".coor" ] },
	{ Extension => "_2",
	  Alias => [ ".vel" ] },
	{ Extension => "_3",
	  Alias => [ ".idx" ] },
	{ Extension => "_4",
	  Alias => [ ".dcd", ".dcd.gz", ".dcd.gz.bad" ] },
	],


# Directory for miscellaneous files (eg. defaults)
    ETC_DIR => "$cgi/etc",

# Minimum age for files to be removed during remote retrieval, in days
# (possibly fractional). Should be at least the frequency with which
# validator is run. CURRENTLY UNUSED.
    REMOVE_MIN_AGE => 1./24,

# Defaults if not specified - not under user control
    DEFAULT_EMAIL => 'nobody@nil.es',

# Defaults if not specified - under user control
    DEFAULT_APP_NAME => 'acemd',
    DEFAULT_NUMSTEPS => 1,
    DEFAULT_PRIORITY => 1,


};

