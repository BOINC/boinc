<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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



require_once('../inc/util.inc');
require_once('../inc/stats_sites.inc');
page_head('Statistics and leaderboards');

echo "
<p>
Statistics for ".PROJECT.":
<ul>
<li><a href=\"top_users.php\">Top participants</a>
<li><a href=\"top_hosts.php\">Top computers</a>
<li><a href=\"top_teams.php\">Top teams</a>
</ul>

<p>
More detailed statistics for ".PROJECT."
and other BOINC-based projects are available
at several web sites:
";
shuffle($stats_sites);
site_list($stats_sites);
echo "
You can get your current statistics in the form
of a 'signature image':
";
shuffle($sig_sites);
site_list($sig_sites);
echo "
You can get your individual statistics
across all BOINC projects from several sites;
see your <a href=\"home.php\">home page</a>.
";

page_tail();
?>
