<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once('../inc/util.inc');
require_once('../inc/translation.inc');

check_get_args(array());

page_head(tra("Rules and policies"));

$show_default = true;

if (function_exists("project_rules_policies")) {
    $show_default = project_rules_policies();
}

if ($show_default) {
echo "

<h3>".tra("Run %1 only on authorized computers", PROJECT)."</h3>
    <p>".tra("Run %1 only on computers that you own, or for which you have obtained the owner's permission. Some companies and schools have policies that prohibit using their computers for projects such as %1.", PROJECT)."

<h3>".tra("How %1 will use your computer", PROJECT)."</h3>
    <p>".tra("When you run %1 on your computer, it will use part of the computer's CPU power, disk space, and network bandwidth. You can control how much of your resources are used by %1, and when it uses them.", PROJECT)."
    <p>".tra("The work done by your computer contributes to the goals of %1, as described on its web site. The application programs may change from time to time.", PROJECT)."

<h3>".tra("Privacy policy")."</h3>
    <p>".tra("Your account on %1 is identified by a name that you choose. This name may be shown on the %1 web site, along with a summary of the work your computer has done for %1. If you want to be anonymous, choose a name that doesn't reveal your identity.", PROJECT)."
    <p>".tra("If you participate in %1, information about your computer (such as its processor type, amount of memory, etc.) will be recorded by %1 and used to decide what type of work to assign to your computer. This information will also be shown on %1's web site. Nothing that reveals your computer's location (e.g. its domain name or network address) will be shown.", PROJECT)."
    <p>".tra("To participate in %1, you must give an address where you receive email. This address will not be shown on the %1 web site or shared with organizations. %1 may send you periodic newsletters; however, you can opt out at any time.", PROJECT)."
    <p>".tra("Private messages sent on the %1 web site are visible only to the sender and recipient.  %1 does not examine or police the content of private messages.  If you receive unwanted private messages from another %1 user, you may add them to your %2 message filter %3.  This will prevent you from seeing any public or private messages from that user.", PROJECT, "<a href=\"edit_forum_preferences_form.php\">", "</a>")."
    <p>".tra("If you use our web site forums you must follow the %2 posting guidelines %3.  Messages posted to the %1 forums are visible to everyone, including non-members.  By posting to the forums, you are granting irrevocable license for anyone to view and copy your posts.", PROJECT, "<a href=moderation.php>", "</a>")."
<h3>".tra("Is it safe to run %1?", PROJECT)."</h3>
    <p>".tra("Any time you download a program through the Internet you are taking a chance: the program might have dangerous errors, or the download server might have been hacked. %1 has made efforts to minimize these risks. We have tested our applications carefully. Our servers are behind a firewall and are configured for high security. To ensure the integrity of program downloads, all executable files are digitally signed on a secure computer not connected to the Internet.", PROJECT)."
    <p>".tra("The applications run by %1 may cause some computers to overheat. If this happens, stop running %1 or use a %2 utility program %3 that limits CPU usage.", PROJECT, "<a href=\"download_network.php\">", "</a>")."
    <p>".tra("%1 was developed by %2. BOINC was developed at the University of California.", PROJECT, COPYRIGHT_HOLDER)."

<h3>".tra("Liability")."</h3>
    <p>".tra("%1 and %2 assume no liability for damage to your computer, loss of data, or any other event or condition that may occur as a result of participating in %1.", PROJECT, COPYRIGHT_HOLDER)."

<h3>".tra("Other BOINC projects")."</h3>
    <p>".tra("Other projects use the same platform, BOINC, as %1. You may want to consider participating in one or more of these projects. By doing so, your computer will do useful work even when %1 has no work available for it.", PROJECT)."
    <p>".tra("These other projects are not associated with %1, and we cannot vouch for their security practices or the nature of their research. Join them at your own risk.", PROJECT);
}
page_tail();
?>
