<?php
require_once("docutil.php");
page_head("Platforms");
echo "
<p>
The computers available to a volunteer computing project
have a wide range of operating systems and hardware architectures.
For example, they may run many versions of Windows
(95, 98, ME, 2000, XP) on many processors variants (486, Pentium, AMD).
Hosts may have multiple processors and/or graphics coprocessors.

<p>
A <b>platform</b> is a compilation target -
typically a combination of a CPU architecture and an operating system.
The BOINC database of each project includes a set of platforms.
Each platform has a <b>name</b> and a <b>description</b> of
the range of computers it can handle.
Each <a href=app.php>application version</a>
is associated with a particular platform.

<p>
For coherence between projects,
you should use only the following platforms:
If you want to add a different platform,
please <a href=contact.php>contact us</a>.
";

list_start();
list_heading("name", "description");
list_item("windows_intelx86", "Microsoft Windows (98 or later) running on an Intel x86-compatible CPU");
list_item("i686-pc-linux-gnu", "Linux running on an Intel x86-compatible CPU");
list_item("x86_64-pc-linux-gnu", "Linux running on an AMD x86_64 or Intel EM64T CPU");
list_item("powerpc-apple-darwin", "Mac OS X 10.3 or later running on Motorola PowerPC");
list_item("i686-apple-darwin", "Mac OS 10.4 or later running on Intel");
list_item("sparc-sun-solaris2.7", "Solaris 2.7 running on a SPARC-compatible CPU");
list_item("sparc-sun-solaris", "Solaris 2.8 or later running on a SPARC-compatible CPU");
list_item("sparc64-sun-solaris", "Solaris 2.8 or later running on a SPARC 64-bit CPU");
list_end();
echo"

<p>
A platform name is compiled into the BOINC client.
The client reports its platform to the scheduling server,
and the scheduling server sends work to a host only
if there is an application version for the same platform.
<p>
In some cases, you may want to associate the a single executable
with multiple platforms.
For example, a Mac/Intel host is able to run Mac/PPC applications
in emulation mode.
If you are unable to compile your application for Mac/Intel,
you can take your Mac/PPC binary and add it as a Mac/Intel app version;
this will allow Mac/Intel hosts to participate in your project.


<h3>Application optimization for specific architectures</h3>

<p>
BOINC allows applications to exploit specific architectures,
but places the burden of recognizing the architecture
on the application.
In other words, if you want to make a version of your application
that can use the AMD 3DNow instruction set,
don't create a new <b>windows_amd_3dnow</b> platform.
Instead, make a version for the <b>windows_intelx86</b> platform
that recognizes when it's running on a 3DNow machine,
and branches to the appropriate code.
<p>
This excludes the combinatorial explosion of versions and architectures
from the internals of BOINC.

<p>
<h3>Web-site statistics breakdown by architecture</h3>
<p>
BOINC collects architecture details about each completed result
to allow detailed statistical breakdowns.

<p>
First, the core client attempts to find
the CPU vendor, the CPU model,
the OS name, and the OS version.
These are stored in the host record.

<p>
Second, applications that recognize even more specific
architecture information can pass it back to the core client
using the <b>boinc_architecture()</b>
function from <a href=api.php>the BOINC API</a>.
This passes a string (project-specific, but typically in XML)
to the core client, which records it in the
<b>architecture_xml</b> field of the <b>result</b> database record.
For example, the application might pass a description like
<pre>
", htmlspecialchars("
<has_3dnow_instructions/>
<graphics_board>ATI Rage 64MB</graphics_board>
"), "
</pre>
This makes it possible, for example, to report average or total
performance statistics for 3DNow hosts contrasted
with other Intel-compatible hosts.

<h3>Tools</h3>
<p>
Platforms are maintained in the <b>platform</b> table in the BOINC DB,
and can be created using the <a href=tool_xadd.php>xadd</a> utility. 
";
page_tail();
?>
