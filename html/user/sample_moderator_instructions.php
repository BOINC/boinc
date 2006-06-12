<?php
require_once("../inc/util.inc");
require_once("../project/project.inc");

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
