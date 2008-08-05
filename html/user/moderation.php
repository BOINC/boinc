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



require_once("../inc/util.inc");
require_once("../inc/forum.inc");
require_once("../project/project.inc");

page_head("Moderation");
echo "
To maximize discussion and flow of information,
our message boards are moderated.
Message board postings are subject to the following posting rules:
".post_rules()."
<p>
Moderators may delete posts that violate any of these rules.
The authors of deleted posts will be notified via email.
Gross offenders may have their ability to post messages temporarily revoked
(though to prevent abuse only project administrators have the ability to do so).
Additional kinds of bad behavior (\"bugging\" posts to trap the
IP addresses of other participants, excessive thread creation to spam
the forums, etc.), while not listed in the formal rules, may still
lead to similar penalties.
<p>
If you think a post violates any of the posting rules,
click the red X on the post and fill out the form;
moderators will be notified of your complaint.
Please use this button only for clear violations - not
personal disputes.
<p>
We try to be as fair as we can when moderating,
but in a large community of users, with many different viewpoints,
there will always be some people that will not be happy
with our moderation decisions.
While we regret that this happens,
please realize that we cannot suit all of the people all of the time
and have to make decisions based on our resources and
what is best for the forum overall.
Please don't discuss our moderation policy on the forums. We aren't
a social engineering project nor are we in the business of creating
a perfectly fair system. So such discussions tend to be counterproductive
and potentially incendiary. If you have a legitimate claim,
send email to the address below.
<p>
This moderation policy is set by the ".PROJECT." project.
If you have comments about the policy, email ".SYS_ADMIN_EMAIL.".

";
page_tail();

?>
