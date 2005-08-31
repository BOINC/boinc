<?php
require_once("docutil.php");
page_head("Versions of BOINC");
echo "

<p>
The BOINC software (including client and server components)
evolve over time.
There are a number of pairwise interactions
in which version mismatches could cause problems:
<ul>
<li> RPC from core client to scheduling server.
<li> RPC from core client to file upload handler.
<li> Interface between core client and application.
<li> Interface between BOINC DB and all BOINC back-end components.
<li> The parsing of the core state file by the core client.
</ul>

<p>
Each BOINC software component has a version
consisting of three integers:
major, minor, and release.

<p>
When a participant updates the core client,
all results currently in progress are discarded
(because new app versions would be needed).
The core client reads the version number from the old
client state file, and discards the results.

<p>
Some changes to the BOINC server software may involve
changes to the BOINC database
(e.g. adding a new table or field).
Such releases will include SQL script for modifying an
existing database in-place.

";
page_tail();
?>
