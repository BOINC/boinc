<?php
require_once("docutil.php");
page_head("Managing the project web site");
echo "
<h2>Forums</h2>
<p>
The Special user feature allows certain users,
like project administrators, developers etc.,
to be shown with that title under their name in the forums.
It is important that people who are new to a project knows
who to pay attention to - and this is a way of giving them a hint.
To enable the feature simply run a query on the forum_preferences table.
You can currently use the following list of titles:

<pre>
\$special_user_bitfield[0]=\"Forum moderator\";
\$special_user_bitfield[1]=\"Project administrator\";
\$special_user_bitfield[2]=\"Project developer\";
\$special_user_bitfield[3]=\"Project tester\";
\$special_user_bitfield[4]=\"Volunteer developer\";
\$special_user_bitfield[5]=\"Volunteer tester\";
\$special_user_bitfield[6]=\"Project scientist\";
</pre>

So if the project administrator has the user number 42
run this query to make him a moderator and project administrator:

<pre>
UPDATE forum_preferences SET special_user=1100000 where userid=42;
</pre>

<h2>Forum moderation</h2>

<h3>Post-level moderation abilities</h3>
<ul>
<li> When browsing posts a moderator is able to delete (or hide really) a
single post by clicking the 'delete' link in the top information bar for
that post.
It is possible to select a reason why the that post was
deleted (this will be displayed to the poster).
Also a message can be
sent directly to the poster's mail address by typing in additional text
in a specific field when deleting.

<li> If a post was deleted by accident it will still be available to
moderators (it now has 'deleted post' in red in the top information bar
for that post).
A moderator can click 'undelete' to make the post
available to the public again.

<li> Moderators can move a single post to another thread by pressing the
top information bar 'move post' link. The moderator will need to know
the destination thread ID. Again a message can be sent to the poster.
<li> TODO: Moderators can set the post rating (do we want this?)

</ul>
<h3>Thread-level moderation abilities</h3>
<ul>
<li> A moderator can delete an entire thread using the 'delete
thread' link located at the very top of the page.
<li> TODO: move threads to other forum
<li> TODO: undelete threads
<li> Users can vote negatively against the first post to eventually mark
the thread as non-interesting (is this known?)
</ul>

<h3>User-level moderation abilities</h3>
<ul>
<li> Users can vote negatively against posts to eventually hide them.
<li> users can ignore other users.
<li> It is possible to list all posts for a user and look at them one by one.
</ul>

<h3>Word-level moderation/filtering abilities</h3>
None, all words are allowed.

<h3>Sub-post-level moderation abilities</h3>
None, moderators are not allowed to edit people's posts.


<h2>Caching</h2>
<h2>News</h2>
";
page_tail();
?>
