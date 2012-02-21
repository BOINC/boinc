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

check_get_args(array());

page_head("Moderator instructions");
echo "
<ul>
<li> Don't let your personal feelings
affect your moderation decisions.
<li> Don't discuss moderation decisions on the forums
of this or other BOINC projects.
Use the email list that has been set up for this purpose.
<li> Don't preemptively moderate.
Except for obscene language or pictures,
avoid deleting a post until someone complains about it.
";
page_tail();

?>
