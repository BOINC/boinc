<?php
require_once('../inc/util.inc');

page_head('Rules and Policies');
echo "

<h3>Run ", PROJECT, " only on authorized computers</h3>

<p>
Run ", PROJECT, " only on computers that you own,
or for which you have obtained the owner's permission.
Some companies and schools have policies that prohibit using their computers
for projects such as ",PROJECT, ".

<h3>How ", PROJECT, " will use your computer</h3>

<p>
When you run ", PROJECT, " on your computer,
it will use part of the computer's CPU power, disk space, and network bandwidth.
You can control how much of your resources are used by ", PROJECT, ",
and when it uses them.

<p>
The work done by your computer contributes to the
goals of ", PROJECT, ",
as described on its web site.
The application programs may change from time to time.

<h3>Privacy policy</h3>

<p>
Your account on ", PROJECT, " is identified by a name that you choose.
This name may be shown on the ", PROJECT, " web site,
along with a summary of the work your computer has done for ", PROJECT, ". 
If you want to be anonymous, choose a name that doesn't reveal your identity.

<p>
If you participate in ", PROJECT, ", information about your computer
(such as its processor type, amount of memory, etc.)
will be recorded by ", PROJECT, " and used to decide
what type of work to assign to your computer.
This information will also be shown on ", PROJECT, "'s web site.
Nothing that reveals your computer's location
(e.g. its domain name or network address) will be shown.

<p>
To participate in ", PROJECT, ", you must give an address where you receive email.
This address will not be shown on the ", PROJECT, " web site or shared with organizations.
", PROJECT, " may send you periodic newsletters;
however, you can opt out at any time.

<h3>Is it safe to run ", PROJECT, " ?</h3>

<p>
Any time you download a program through the Internet you are taking a chance:
the program might have dangerous errors,
or the download server might have been hacked.
", PROJECT, " has made efforts to minimize these risks.
We have tested our applications carefully.
Our servers are behind a firewall and are configured for high security.
To ensure the integrity of program downloads,
all executable files are digitally signed on a secure computer
not connected to the Internet.

<p>
The applications run by ", PROJECT, " may cause some computers to overheat.
If this happens, stop running ", PROJECT ," or use a
<a href=download_network.php>utility program</a> that limits CPU usage.

<p>
", PROJECT, " was developed by ".COPYRIGHT_HOLDER.".
BOINC was developed at the University of California.

<h3>Liability</h3>

<p>
", PROJECT, " and ".COPYRIGHT_HOLDER."
assume no liability for damage to your computer,
loss of data, or any other event or condition that may occur
as a result of participating in ", PROJECT, ".

<h3>Other BOINC projects</h3>

<p>
Other projects use the same platform, BOINC, as ", PROJECT, ".
You may want to consider participating in one or more of these projects.
By doing so, your computer will do useful work even when ", PROJECT, " has no work available for it.

<p>
These other projects are not associated with ", PROJECT, ", and we cannot vouch for their security practices or the nature of their research.
Join them at your own risk.
";
page_tail();
?>
