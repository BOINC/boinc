<?php
require_once('../inc/util.inc');
require_once('../inc/stats_sites.inc');
page_head('Statistics and leaderboards');

echo "
<p>
Statistics for ".PROJECT.":
<ul>
<li><a href=top_users.php>Top participants</a>
<li><a href=top_hosts.php>Top computers</a>
<li><a href=team.php>Top teams</a>
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
see your <a href=home.php>home page</a>.
";

page_tail();
?>
