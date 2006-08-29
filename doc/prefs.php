<?php
require_once("docutil.php");
page_head("Preferences");
echo "
<p>
You can specify <b>preferences</b> that limit
when and how BOINC uses your computers.
Preferences are divided into two groups: General and Project.

<h2>Editing preferences</h2>
<p>
You can view and edit your preferences via the project's web site.
Click on 'Your account', then 'View or edit general preferences'.
<blockquote>
<b>
Note: these links may be different on some projects.
For example, on Climateprediction.net you must click
'My CPDN', then 'BOINC CPDN', then 'Your account',
and 'View or edit general preferences'.
</b>
</blockquote>
This shows you the preferences.
If you want to change anything,
click on 'Edit preferences'.

<p>
When you change your preferences on the web,
the changes won't take effect immediately on your computer;
they'll take effect the next time your computer
connects to the project's server.
If you want this to happen immediately,
bring up the BOINC Manager on your computer,
select the project, and click 'Update'.

<p>
If you're running BOINC on several computers,
preference changes will eventually propagate to all of them.

<p>
If you participate in multiple BOINC projects,
click <a href=multiple_projects.php>here</a>
for information about preferences.

<h2>Location-specific preferences</h2>

If you have computers at several locations (e.g. home, work and school)
you may want to use different preferences for different locations.
The preferences editing system (see above)
lets you create (or delete) separate preferences for home, work, and school.

<p>
Each computer attached to your account has a location.
To view this, go to the project's web site, then click
'Your account' and 'View Computers'.
Click on the ID of the computer you're interested in.
At the bottom of the page there's a popup menu
that lets you see or change the location.
A change to a computer's location will take effect
only when that computer contacts the server;
you can make this happen immediately
using the BOINC Manager's Update command.

<p>
If a computer has its location set to 'home' (for example),
and you've defined separate preferences for home,
it will use those preferences.
Otherwise it will use your default preferences.

<p>
Your account has a 'default location' (home, work, or school).
New computers attaching to your account will be given the default location.
The default location is part of your project preferences,
so to change it, edit your project preferences.

<h2>General preferences</h2>
<b>General preferences</b> apply to all BOINC projects in which you participate.
They include:
";

list_start();

list_bar("Processor usage");
list_item("When to work",
"You can specify whether computation should be done
1) if the computer is in use (i.e. during keyboard and mouse input);
2) if the computer is being powered by batteries (for laptop users).
You can also specify a range of hours when work should be done.
");


list_item("Leave applications in memory while preempted",
    "If yes, applications will be preempted by suspending and resuming,
    rather than quitting.
    This uses more virtual memory, but uses CPU time more efficiently."
);

list_item("Switch between applications every X minutes",
    "This determines how often BOINC switches between projects."
);
list_item("Maximum number of processors to use",
    "On a multiprocessor, this limits the number of processors
    that BOINC will use."
);
list_item("Use at most X% of CPU time",
    "It you specify 50%, BOINC will compute only every other second.
    This reduces the heat output and energy usage of your CPU chip."
);

list_bar("Disk and memory usage");

list_item("Usage limits",
"You can limit the disk space used by BOINC in any of three ways:
1) Maximum disk space used by BOINC;
2) Maximum percentage of total space that can be used by BOINC.
3) Minimum disk space to keep free.
");

list_item("Access interval",
    "A suggested interval between disk accesses.
    Useful on laptops where the disk may be spun down for long periods.
    "
);

list_item("Virtual memory",
    "Limit the virtual memory used by BOINC"
);

list_bar("Network usage");
list_item("Time of day limits",
    "Limit the hours during which BOINC will do network communication."
);
list_item("Bandwidth limits",
    "Limit the number of bytes per second uploaded or downloaded by BOINC."
);
list_item("Network connection preferences",
"Whether to wait for confirmation before making network connections,
and whether to disconnect when done.");

list_item("Time between network connections",
    "Target time between network connections.
    When your computer asks a server for work,
    it will try to get enough work to last for this long."
);

list_end();

echo "

<h2>Project preferences</h2>
There is a separate set of <b>project preferences</b>
for each project in which you participate.
These include:
";
list_start();
list_item(
    "Resource share",
    "If projects contend for resources,
    the amount allocated to a project is proportional to this number."
);
list_item(
    "Email prefs",
    "Whether the project should send you newsletters by email."
);
list_item(
    "Hide computer information",
    "Whether the project should show information
    about your computers
    (their CPU and OS type, benchmark ratings etc.;
     not their names or addresses)
    on its web site."
);
list_item(
    "Default computer location",
    "The location assigned to computers that attach to this account."
);
list_item(
    "Project-specific preferences",
    "Defined by the project;
    e.g., to specify graphics color schemes."
);
list_end();
echo"
<p>

";
page_tail();
?>
