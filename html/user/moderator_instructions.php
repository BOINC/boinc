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

page_head("Moderator instructions");
echo "
Your job as a moderator is to enforce the following rules:
".post_rules()."
Delete posts or threads <b>only if they clearly violate
one or more of these rules</b>.
However, there may be other cases where it is alright to
delete posts or threads as the moderator sees fit, 
such as:
<ul>
<li>whole threads that have disintegrated into pointless
argument, especially between a few parties, though the
content might not exactly violate any of the rules above
<li>older threads (that don't appear on current first page)
that may contain less offensive but still questionable material
<li>a thread needs massive cleanup can be temporarily
deleted to hide it while cleanup takes place
<li>clear invasions of privacy (bugging posts to trap
IP addresses, posting contents of personal emails (real
or fake), etc.)
</ul>
If in doubt, discuss it with other moderators on the email list.
Please respect the following guidelines:
<ul>

<li> Don't let your personal opinions
or moods affect your moderation decisions.
You may not delete a post simply because
you disagree with it or dislike its author.
If you find yourself getting angry, take a break.

<li> Don't discuss moderation decisions on the forums
of this or other BOINC projects.
Use the email list that has been set up for this purpose.
Consider using an anonymous account for moderation,
so that your own forum activities are kept separate from
your moderation duties.

<li>
Except for posts containing obscene language or pictures,
avoid deleting posts until someone complains about them
(and, of course, don't delete them simply because someone has complained).

</ul>
<p>
It is possible for a project administrator to temporarily banish
users by selecting the \"banish author\" link in each post.
In this case each offending author is sent an e-mail stating
he/she will be unable to post for two weeks.
Moderators will not have access to the \"banish author\" button.
If they feel a person should be banished, they should request
that a project administrator do so.
<p>
<b>REMEMBER:</b>
It is impossible to keep any internet forum free of yahoos.
People will say incredibly stupid things when nobody can
punch them in the face. Don't try to solve this problem -
just try to maintain some level of sanity.
";
page_tail();

?>
