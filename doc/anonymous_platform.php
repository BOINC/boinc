<?
require_once("docutil.php");
page_head("Compile-it-yourself BOINC");
echo "
BOINC applications, and the BOINC core client, are native-mode programs,
so different versions are required for each platform
(a 'platform' is the combination of an operating
system and a processor type: e.g., Linux/IntelX86).
BOINC provides two ways to deal with the problem
of making programs available for different platforms.

<h3>The project-compiles-it-for-you model</h3>
<p>
In this approach, the BOINC-based project
compiles program versions for common platforms
(Windows/Intel, Linux/Intel, Mac OS/X. etc.),
and places them on its servers.
A participant downloads the core client for his platform
(assuming that it's supported by the project).
When the core client requests work from the project's scheduling server,
it tells the server its platform,
and the scheduling server instructs it to download
the appropriate application executables.
<p>
Although this addresses the needs of most BOINC participants,
there are two groups for whom it is inadequate:
<ul>
<li>
People who, for security reasons,
want to only run executables they have compiled themselves.
<li>
People whose computers have platforms not supported by the project
(projects are generally resource-limited and cannot support all platforms).
</ul>

<h3>The compile-it-yourself model</h3>
<p>
In this model participants compile programs themselves
rather than downloading them from the project.
Here's how it works:

<ul>
<li>
Download the source code for the BOINC core client,
and for the project's applications,
and compile them for your computer.
Alternatively, you might download the programs from
a server of your choosing.
<li>
Run the core client with a option telling
it where to find the applications;
it incorporates them
into its state file and directory structure,
as if it had been downloaded from the server.
<li>
When the core client requests work from the scheduling server,
it reports its platform as 'anonymous',
and provides a list of the applications it has.
The server then just sends whatever work is available for those applications.
</ul>

This model is possible only with projects that make  their application
source code available.
<p>
<b>
Note: the compile-it-yourself model is under development,
and is not currently available.
Participants with uncommon platforms can prepare by porting
the current core client and applications to their platforms.
</b>
";
page_tail();
?>
