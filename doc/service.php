<?php
require_once("docutil.php");
page_head("Running BOINC as a Windows Service");
echo "
<p>
BOINC can be run as a Windows service.
This requires the command-line interface (CLI) version of the core client,
which is not available for download;
you'll have to build it from the source code
using Visual Studio .Net.

<p>
If you haven't already run BOINC on the machine,
you'll need to run through the setup procedure
(using either the CLI or the GUI client)
in order to establish the
account* files which are needed for the project URLs and authenticators.
<p>
Then put the CLI executable (boinc_cli.exe) into the BOINC folder.
Type this from the commandline:

<pre>
boinc_cli -install
</pre>

This will setup BOINC as a Windows service which can be started on boot
and will be hidden from view.
If you are executing this on a Windows 2003 machine
the default user account that is chosen is 'Network
Service' which means you'll need to grant Read/Write/Execute/Delete permissions
to the client folder and all of its children before attempting to start
the service.
On Windows XP and older systems the client currently sets
itself up as local system.
<p>
Messages are logged to the 'eventlog' - check there
periodically for error and status messages.

";
page_tail();
?>
