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
