<?php
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
It is possible for an administrator to temporarily banish
users by selecting the \"banish author\" link in each post.
In this case each offending author is sent an e-mail stating
he/she will be unable to post for two weeks.
Moderators will also have the \"banish author\" button but
should only use it in emergency cases, such as:
<ul>
<li>spammers suddenly littering the forums with garbage
at a quick rate, or
<li>egregious offenders (i.e. highly violent or pornographic
images not suitable to children under 18 years old). 
</ul>
In normal cases, if a moderator wants to banish a specific
author they must ask the adminstrator first. <b>Any</b>
user banishment results in an e-mail to the internal
moderation list describing who got banished and why.
<p>
<b>REMEMBER:</b>
It is impossible to keep any internet forum free of yahoos.
People will say incredibly stupid things when nobody can
punch them in the face. Don't try to solve this problem -
just try to maintain some level of sanity.
";
page_tail();

?>
