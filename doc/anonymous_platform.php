<?php
require_once("docutil.php");
page_head("Compiling BOINC client software yourself");
echo "
BOINC applications, and the BOINC core client, are native-mode programs,
so different versions are required for each platform
(a 'platform' is the combination of an operating
system and a processor type: e.g., Linux/IntelX86).

<p>
BOINC-based projects compile program versions for common platforms
(Windows/Intel, Linux/Intel, Mac OS/X. etc.),
and place them on its servers.
A participant downloads the core client for his platform
(assuming that platform is supported by the project).
When the core client requests work from the project's scheduling server,
the client tells the server its platform,
and the server instructs it to download the appropriate executables.
<p>
This addresses the needs of most BOINC participants,
but it's inadequate if:
<ul>
<li>
your computers have platforms not supported by the project;
<li>
for security reasons,
you want to only run executables you have compiled yourself;
<li>
you want to optimize applications for particular architectures.
</ul>

<p>
To handle these cases, BOINC lets you compile programs yourself
rather than downloading them from the project.
Here's how it works:

<ul>
<li>
<a href=community.php>Download</a> the source code for the BOINC core client
and the project's applications, and compile them on your computer
(instructions for compiling the core client are
<a href=build_client.php>here</a>).
Or download executables from a server of your choosing.
<li>
Run the core client and attach to the project.
This will create a 'project directory'
(whose name is the project URL) in the BOINC directory.
Exit the client.
<li>
Create a file <b>app_info.xml</b> in the project directory.
This file lists the applications you have compiled or downloaded.
It has the following form:
<pre>", htmlspecialchars("
<app_info>
    <app>
        <name>setiathome</name>
    </app>
    <file_info>
        <name>setiathome_2.18_windows_intelx86.exe</name>
    </file_info>
    <app_version>
        <app_name>setiathome</app_name>
        <version_num>218</version_num>
        <file_ref>
            <file_name>setiathome_2.18_windows_intelx86.exe</file_name>
            <main_program/>
        </file_ref>
    </app_version>
</app_info>
"), "</pre>
<li>
Run the core client again.
When it requests work from the scheduling server,
it will report its platform as 'anonymous',
and provides a list of the applications it has.
The server then sends whatever work is available for those applications.
</ul>

This model is possible only with projects that make their application
source code available.

<p>
You may want to check out the following email lists
(e.g. the port may already exist):
<ul>
<li>
<a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_opt>boinc_opt@ssl.berkeley.edu</a>: discussion of porting and optimization of BOINC applications.
<li>
<a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_dev>boinc_dev@ssl.berkeley.edu</a>: discussion of development and porting of BOINC software.
</ul>
<p>
";
page_tail();
?>
