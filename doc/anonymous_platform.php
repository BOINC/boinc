<?php
require_once("docutil.php");
page_head("Make your own client software");

echo "
<p>
BOINC applications, and the BOINC core client, are native-mode programs, so
different versions are required for each platform (a 'platform' is the
combination of an operating system and a processor type: e.g.,
Linux and Intel/x86).

<p>
BOINC-based projects compile program versions for common platforms
(Windows/Intel, Linux/Intel, Mac OS/X. etc.), and place them on its servers.
A participant downloads the core client for his platform (assuming that
platform is supported by the project).  When the core client requests work
from the project's scheduling server, the client tells the server its
platform, and the server instructs it to download the appropriate
executables.
<p>
This addresses the needs of most BOINC participants, but it's inadequate if:
<ul>
<li>
your computers have platforms not supported by the project;
<li>
for security reasons, you want to only run executables you have compiled
yourself;
<li>
you want to optimize applications for particular architectures.
</ul>

<p>
To handle these cases, BOINC lets you make or obtain
the client software yourself rather than downloading it.
<p>
First, get the BOINC client software for your computer.
You can do this in either of two ways:


<ul>
<li>
<a href=compile.php>Download and compile</a>
the BOINC client software on your computer.
<li>
If available, download BOINC executablers for your
computer from a third-party source.
A list of such sources is <a href=download_other.php>here</a>.
</ul>

Second, get the project's application for your computer.
Again, you can either compile it yourself
or download the executable from a third party.
The details vary between projects,
and not all projects make their source code available.
As an example, instructions for SETI@home are
<a href=http://setiweb.ssl.berkeley.edu/sah_porting.php>here</a>.

<p>
Finally:
<ul>
<li>
Run the core client and attach to the project.  This will create a
'project directory' (whose name is the project URL) in the BOINC
directory.  Exit the client.

<li>
Create a file <b>app_info.xml</b> in the project directory.
This file lists the applications you have compiled or downloaded.
It has the following form:
".html_text("
<app_info>
    <app>
        <name>setiathome</name>
    </app>
    <file_info>
        <name>setiathome_4.07_windows_intelx86.exe</name>
        <executable/>
    </file_info>
    <app_version>
        <app_name>setiathome</app_name>
        <version_num>407</version_num>
        <file_ref>
            <file_name>setiathome_4.07_windows_intelx86.exe</file_name>
            <main_program/>
        </file_ref>
    </app_version>
</app_info>
")."
where 407 is the application's version number.
<li>
Run the core client again.  When it requests work from the scheduling
server, it will report its platform as 'anonymous', and provides a list of
the applications it has.  The server then sends whatever work is available
for those applications.
</ul>

<p>
This model is possible only with projects that make their application source
code available.
<p>
You may want to check out the following email lists (e.g. the port may
already exist):
<ul>
<li>
<a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_opt>
boinc_opt@ssl.berkeley.edu</a>:
discussion of porting and optimization of BOINC applications.
<li>
<a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_dev>
boinc_dev@ssl.berkeley.edu</a>: discussion of development and porting of
BOINC software.
<li>
<a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_cvs>
boinc_cvs@ssl.berkeley.edu</a>: CVS checkins to the BOINC
source are reported here.
</ul>

";
page_tail();
?>
