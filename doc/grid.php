<?php
require_once("docutil.php");
page_head("Integrating BOINC projects with Grids");
echo "
<p>
Researchers are CERN have set up a system
where submitted jobs are sent either to a BOINC project
or to a GRAM job manager.
They developed two utilities,
kill_wu and poll_wu, to support this.
They are in the boinc/tools directory.
Contact Christian S&oslash;ttrup (chrulle at fatbat.dk) for more info.
<p>
The <a href=http://lattice.umiacs.umd.edu/>Lattice</a> project
from the University of Maryland
is developing a Grid system that integrates Globus,
BOINC, and several other software components.
";
page_tail();
?>
