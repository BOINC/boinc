<?
require_once("docutil.php");
page_head("Preferences");
echo "
<p>
You can specify <b>preferences</b> determining and limiting
how BOINC uses your computers.
Preferences are divided into two groups: 
<h3>General preferences</h3>
<b>General preferences</b> apply to all BOINC projects in which you participate.
They include:
<ul>
<li> Whether work (computation and network transfer) should be done
while the host is being used (i.e. during keyboard and mouse input).
<li> Whether work should be done while the computer is being powered
by batteries (for laptop users).
<li> Whether to wait for user confirmation before making network connections.
<li> Min and max days of work to keep on disk.
If the host is frequently disconnected from the Internet, the min
should be at least as long as the typical period of disconnection.
The larger the difference between min and max, the less often
the BOINC client will connect to the Internet.
<li> Maximum disk space used by BOINC.
<li> Maximum percentage of free space used by BOINC.
<li> Minimum disk space to keep free.
</ul>
You can view and edit your global preferences through a web interface,
at the site of any project in which you participate.
Changes are automatically propagated to all your hosts;
this is done the next time the host contacts the project's server,
so there may be some delay.

<h3>Project preferences</h3>
There is a separate set of <b>project preferences</b>
for each project in which you participate.
They include:
<ul>
<li> Resource share: if projects contend for resources,
the amount allocated to a project is proportional to this number.
<li> Whether to show email address on web site
<li> Whether project should send emails to user
<li> Project-specific preferences (defined by the project;
e.g., to specify graphics color schemes).
</ul>
<p>
You can view and edit project preferences through
a web interface at the project's web site.

<h3>Location-specific preferences</h3>
If you have computers both at home and at work
you may want to use differences preferences for them.
In addition to your 'primary preferences'
(which are used by default)
BOINC allows you to create separate preferences for
home, work, and school.
<p>
Your account with a project has a 'default location'
(home, work, or school).
New computers registered to your account will be
given the default location.
You can change the location of an existing computer
through the project's web site.
";
page_tail();
?>
