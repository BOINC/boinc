<?php
    require_once("util.inc");
    require_once("db.inc");

    db_init();
    $user = get_user_from_cookie();
    printf(
	"<head>\n"
	."<title>".PROJECT." Distributed Computing Project</title>\n"
	."</head>\n"
	."<body text=#000000 link=#0000cc vlink=#551a8b alink=#ff0000>\n"
	."<table width=780>\n"
	."<tr><td><h1 align=center>".PROJECT." Distributed Computing Project</h1></td></tr>\n"
    );
    if ($user) {
        echo "<tr><td><h3>Welcome $user->name</h3>\n";
        echo "If you are not $user->name or would wish to log in as another user ";
        echo "<a href=login.php>login here</a>.";
    } else {
	echo "<tr><td>\n";
    }
?>

<p>
This distributed computing project is running on the BOINC software platform.
BOINC is a software platform for public-participation distributed
computing projects.
Users are allowed to simultaneously participate in multiple projects
and to choose how to allocate their resources for each project. 

<h3>Joining this project</h3>
First, <a href=create_account.php>create an account</a>.
You will be sent an authenticator
to the email specified.
After successfully creating an account,
<a href=download.php>download the BOINC client</a>.
Install and run the client.
When it asks you for authenticator,
cut and paste the authenticator from the email.

<ul>
<li><a href=create_account.php>Create account</a></li>
<li><a href=download.php>Download core client</a></li>
<li><a href=login.php>Login</a></li>
<li><a href=home.php>User home page</a></li> - view stats, modify preferences
<li><a href=team.php>Teams</a></li> - join sample distributed computing project community
</ul>
</td></tr></table>
</body>
