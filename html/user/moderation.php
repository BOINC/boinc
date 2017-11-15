<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

// This page is intended for forum readers (not necessarily posters).
// It's linked to from the bottom of forum pages.

require_once("../inc/util.inc");
require_once("../inc/forum.inc");
require_once("../project/project.inc");

page_head("Message board moderation");
echo
tra("Our message boards are moderated. Posts are subject to the following rules:")
.post_rules()
."<p>"
.tra("If you think a post violates any of the rules, you can notify moderators by clicking the red X below the post and filling out the form.")
."<p>"
.tra("This moderation policy is set by the %1 project.  If you have comments about the policy or its enforcement, email %2.",
    PROJECT, SYS_ADMIN_EMAIL
);
page_tail();

?>
