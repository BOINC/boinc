<?
require_once("docutil.php");
page_head("Compiling applications yourself");
echo "
BOINC applications, and the BOINC core client, are native-mode programs,
so different versions are required for each platform
(a 'platform' is the combination of an operating
system and a processor type: e.g., Linux/IntelX86).
BOINC provides two ways to deal with the problem
of making programs available for different platforms.

<h3>The project-compiled model</h3>
<p>
In this approach, the BOINC-based project
compiles program versions for common platforms
(Windows/Intel, Linux/Intel, Mac OS/X. etc.),
and places them on its servers.
A participant downloads the core client for his platform
(assuming that it's supported by the project).
When the core client requests work from the project's scheduling server,
the client tells the server its platform,
and the server instructs it to download
the appropriate executables.
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
Or download the programs from a server of your choosing.
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
";
page_tail();
?>
