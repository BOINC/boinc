<?
require_once("docutil.php");
page_head("Platforms");
echo "
<h3>Goals</h3>
<p>
BOINC is intended to accommodate participant hosts
with a wide range of operating systems and hardware architectures.
For example, the hosts may run many versions of Windows
(95, 98, ME, 2000, XP) on many processors
(486, Pentium, AMD) with many architectural extensions
(ATI or NVidia graphics coprocessors).
BOINC addresses the following goals:
<ul>
<li> The system should avoid sending code that the host can't execute.

<li> Applications should be able to exploit specific architectural features,
i.e. to use nonstandard instructions or coprocessors if available.
<li>
Enough architectural information should be stored on the server
so that statistics can be broken down
according to specific features.
<li> Simplicity.
The combinatorial explosion of versions and architectures should be
excluded from the internals of BOINC.
</ul>

<h3>Design</h3>
<p>
A <b>platform</b> is a compilation target.
A set of platforms is maintained in the BOINC database of each project.
Each platform has a <b>name</b> and a <b>description</b> of
the range of architectures it can handle.
Each BOINC program (core client and application) is linked to a platform.
<p>
At the minimum, a platform is a combination
of a CPU architecture and an operating system.
Examples might include:

<p>
<table cellpadding=8 border=1>
<tr><th>name</th><th>description</th></tr>
<tr><td>windows_intelx86</td><td>Microsoft Windows (95 or later) running on an Intel x86-compatible processor</td></tr>
<tr><td>linux_xxx</td><td>Linux running on an Intel x86-compatible processor</td></tr>
<tr><td>macos_ppc</td><td>Mac OS 9.0 or later running on Motorola PowerPC</td></tr>
<tr><td>sparc_solaris</td><td>Solaris 2.1 or later running on a SPARC-compatible processor</td></tr>
</table>

<p>

The name of a platform should specify a particular version (e.g. of an OS)
only if it uses features new to that version.
For example, the platform <b>sparc_solaris2.8</b> should apply
to SPARC machines running Solaris 2.8 or greater.

<p>
For simplicity, platforms are assumed to be mutually exclusive:
i.e. an application for platform X is not assumed to work
on a host running core client platform Y if X <> Y.
The BOINC scheduling server will send work to a host only
if there is an application version for the same platform.

<p>
There should be as few platforms as possible.
For example, suppose that there are both Solaris2.6
and Solaris2.7 platforms.
Then any host running the Solaris2.6 core client will
only be able to run Solaris2.6 applications.
Application developers will have to create versions
for both 2.6 and 2.7, even if they're identical.

<p>
<h3>Application optimization for specific architectures</h3>

<p>
BOINC allows applications to exploit specific architectures,
but places the burden of recognizing the architecture
on the application.

<p>
In other words, if you want to make a version of your application
that can use the AMD 3DNow instruction set,
<i>don't</i> create a new <b>windows_amd_3dnow</b> platform.
Instead, make a version for the <b>windows_intelx86</b> platform
that recognizes when it's running on a 3DNow machine,
and branches to the appropriate code.

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
architecure information can pass it back to the core client
using the <b>boinc_architecture()</b>
function from <a href=api.php>the BOINC API</a>.
This passes a string (project-specific, but typically in XML)
to the core client, which records it in the
<b>architecture_xml</b> field of the <b>result</b> database record.
For example, the application might pass a description like
<pre>
&lt;has_3dnow_instructions/>
&lt;graphics_board>ATI Rage 64MB</graphics_board>
</pre>
This makes it possible, for example, to report average or total
performance statistics for 3DNow hosts contrasted
with other Intel-compatible hosts.


<h3>Avoiding platform anarchy</h3>
<p>
Each BOINC project is free to create its own platforms.
To avoid anarchy, however, we recommend that
platform creation and naming be coordinated by a single group
(currently the SETI@home project at UCB).


<h3>Tools</h3>
<p>
Platforms are maintained in the <b>platform</b> table in the BOINC DB,
and can be created using the <a href=tools_other.php>add</a> utility. 
";
page_tail();
?>
