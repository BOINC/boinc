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
Each BOINC software component has a version number
of the form 100*X + Y,
where X and Y are the major and minor versions.

<p>
In general, all the parts of a BOINC system must
have the same major version.
A core client can interact with servers,
and can execute applications,
only if they have the same major version.

<p>
Major-version changes to the BOINC software will require
that all projects update their server software (and databases)
and that all participants update their core client software on all hosts.
This doesn't have to happen all at once,
but in the midst of the crossover
some clients won't be able to access some servers.

<p>
When a project makes a major-version change in its server software,
it may need to create new versions of its applications.
It must invalidate (by incrementing min_version)
all app versions that are incompatible with the new server software.

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
