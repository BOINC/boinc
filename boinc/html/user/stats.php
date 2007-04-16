<?php
require_once('../inc/db.inc');
require_once('../inc/util.inc');
db_init();
page_head('Project statistics');

echo "
<h2>Project statistics</h2>
<p>
<b>Leader boards</b> (showing which
users, teams, and computers have done the most work)
are not directly available on this web site.
Instead, the raw data is available as compressed XML files.
The format is described
<a href=http://boinc.berkeley.edu/db_dump.php>here</a>,
and the files are
<a href=stats/>here</a>.

<p>
This data can be summarized and represented as Web pages.
Examples:
<ul>
<li>
<a href=http://www.boinc.dk/index.php?page=statistics>http://www.boinc.dk</a>,
developed by <a href=mailto:stats@boinc.dk>Janus Kristensen</a>.
<li>
<a href=http://www.saschapfalz.de/boincstats/boinc-stats.php>boincstats</a>,
developed by Sascha Pfalz.
</ul>
Both systems are implemented using PHP.
If you are interested in participating in these efforts,
please contact the developers.
";

page_tail();
?>
