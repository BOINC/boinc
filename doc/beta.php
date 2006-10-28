<?php
require_once("docutil.php");
page_head("Beta-test applications");
echo "
It's important to test new applications on a wide range of hosts,
since bugs may appear only with particular OS versions,
memory sizes, display types, usage patterns, and so on.
It's handy to use volunteers to do this testing,
since they provide the needed diversity of hosts.
<p>
One way to implement this is to create a separate test project.
This has two disadvantages:
<ul>
<li> There is overhead in creating and maintaining a separate project.
<li> The credit accrued by testers goes to a different project.
</ul>
BOINC provides a way to do beta testing in the context
of your existing project.
You can let users volunteer to run test applications,
warning them in advance that these applications are more likely to crash.
These users will get a mixture of regular and test results,
and they'll get credit for both.
Here's how to do it:
<ul>
<li> Upgrade to the BOINC server software of Oct 25 2006 or later.
<li> Create a new <a href=app.php>application</a>
using <a href=tool_xadd.php>xadd</a>.
Include &lt;beta&gt;1&lt;/beta&gt; in the &lt;app&gt; element to
designate it as a beta-test application.
<li> Add a validator and assimilator for the test application.
<li> Include the line
<pre>
\$project_has_beta = true;
</pre>
in your html/project/project_specific_prefs.inc file.
This will add a 'Run test applications?' option to your
project-specific preferences.
<li> Publicize this on your web site and wait for
some users to set their preferences to allow test apps.
(Note: this flag is stored in XML in the project_prefs
field of the user table; scan for
&lt;allow_beta_work&gt;1&lt;/allow_beta_work&gt;).
<li> Create application versions for the test application.
<li> Create work (as needed for testing) for the test application.
To prevent test work from dominating regular work,
either use the -allapps feeder option
(and give the test app a small weight)
or make a work generator for the test app that maintains
only a small number of unsent results.

</ul>

";
page_tail();
?>
