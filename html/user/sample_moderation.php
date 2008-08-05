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
require_once("../project/project.inc");

page_head("Moderation");
echo "
To maximize discussion and flow of information,
our message boards are moderated.
Message board postings are subject to several <b>posting rules</b>,
listed in the post-message page.
Moderators may delete posts that violate any of these rules.
The authors of deleted posts will be notified via email.

<p>
If you think a post violates any of the posting rules,
click the red X on the post and fill out the form;
moderators will be notified of your complaint.

<p>
The moderation policy is set by the ".PROJECT." project.
If you have comments about the policy, email ".SYS_ADMIN_EMAIL.".

";
page_tail();

?>
