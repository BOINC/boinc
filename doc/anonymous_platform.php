<?php
require_once("docutil.php");
page_head("Make your own client software");

echo "
<p>
BOINC applications, and the BOINC core client, are native-mode programs, so
different versions are required for each platform
(a 'platform' is the combination of an operating system and a processor type:
e.g., Linux/Intel).

<p>
The BOINC core client is available for common platforms
(Windows/Intel, Linux/Intel, Mac OS/X. etc.) from
<a href=download.php>this web site</a>.
BOINC-based projects compile program versions for some or all of these platforms
and place them on their servers.
Typically, you download the BOINC core client version for your platform.
When the core client requests work from the project's scheduling server,
the client tells the server its platform,
and the server instructs it to download the appropriate program version.
<p>
This addresses the needs of most BOINC participants, but it's inadequate if:
<ul>
<li>
your computers have platforms not supported by BOINC
or by the project;
<li>
for security reasons, you want to only run executables you have compiled
yourself;
<li>
you want to optimize applications for particular architectures.
</ul>

<p>
To handle these cases, BOINC lets you make the client software yourself,
or obtain it from a third party,
rather than downloading it from its 'official' source.
This applies to both the core client and to project-specific applications.

<h3>BOINC client software</h3>
<p>
You can get the BOINC client software in any of three ways:

<ul>
<li> <a href=download.php>Download an executable</a> it from this web site.
<li>
<a href=compile_client.php>Download the source code and compile it yourself</a>
on your computer.
<li>
Download BOINC executables for your
computer from a third-party source.
A list of such sources is <a href=download_other.php>here</a>.
</ul>

<h3>Project-specific applications</h3>
<p>
Not all BOINC projects make their source code available.
The following applies only to projects that make their source code available.
As an example, instructions for SETI@home are
<a href=http://setiweb.ssl.berkeley.edu/sah_porting.php>here</a>.

<p>
<ul>
<li>
Either get the application source code and compile it yourself,
or download an executable from a third party.

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
<b>
Note: if you decide to switch back to using the project-supplied
executables, you must delete the app_info.xml file, then reset the project.
</b>
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
