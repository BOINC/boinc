<?php
require_once("../inc/util.inc");
require_once("../inc/forum.inc");
require_once("../project/project.inc");

page_head("Moderation");
echo "
To maximize discussion and flow of information,
our message boards are moderated.
Message board postings are subject to the following posting rules:
".post_rules()."
Moderators may delete posts that violate any of these rules.
The authors of deleted posts will be notified via email.

<p>
If you think a post violates any of the posting rules,
click the red X on the post and fill out the form;
moderators will be notified of your complaint.

<p>
This moderation policy is set by the ".PROJECT." project.
If you have comments about the policy, email ".SYS_ADMIN_EMAIL.".

";
page_tail();

?>
