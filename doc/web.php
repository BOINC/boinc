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
Integer / Title
1 Project Administrator
2 Forum moderator
3 Developer
4 Volunteer developer
5 Project Scientist
</pre>

So if the project administrator has the user number 42
simply run this query to make him a special user
(provided that the row exists):

<pre>
UPDATE forum_preferences SET special_user=1 where userid=42 
</pre>

<h2>Caching</h2>
<h2>News</h2>
";
page_tail();
?>
