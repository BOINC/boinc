<head>
<title>Sample Distributed Computing Project</title>
</head>
<body text=#000000 link=#0000cc vlink=#551a8b alink=#ff0000>
<h1>Sample Distributed Computing Project</h1>

<?php
    require_once("util.inc");
    require_once("db.inc");
    db_init();
    $user = get_user_from_cookie();
    if ($user) {
        echo "<h3>Welcome $user->name</h3>";
        echo "If you are not $user->name or would wish to log in as another user ";
        echo "<a href=login.php>login here</a>.";
    }
?>

<p>
This distribued computing project is running on the BOINC software platform. BOINC is a software platform for public-participation distributed computing projects.  Users are allowed 
to simultaneously participate in multiple projects and to choose how to allocate their resources
for each project. 

<h3>Joining this project</h3>
First, <a href=create_account.php>create an account</a>, setting your preferences to specify how much your computer should be 
working on this project. You may view your user page to see how much credit you have accumulated as your 
participation proceeds.  In addition, you may update your user information or modify your preferences 
through your user page at anytime to reallocate your resources among the different projects you may 
be involved in. 
<p>
When creating you account the project will send you an authenticator to the email specified.  After successfully 
creating an account <a href=download.php>download the core client</a>.  In order to run the core client you must cut and paste in the 
authenticator provided in the email sent to you by the project.
<h3>Joining other projects</h3>

When you join subsequent projects make sure to edit your project preferences for your home project 
(the first BOINC supported project you joined) accordingly.  You will need to update project preferences with each new project you join.  You will be asked to provide the authenticator for each 
project so make sure to cut and paste those in from the emails sent to you.  This will inform other projects 
of the other projects you are involved in, allowing you to participate in all of them simultaneously.
<p>
If this is not the first project you are joining, make sure to edit the project preferences of your home 
project.

<ul>
<li><a href=create_account.php>Create account</a></li>
<li><a href=download.php>Download core client</a></li>
<li><a href=login.php>Login</a></li>
<li><a href=home.php>User home page</a></li> - view stats, modify preferences
<li><a href=team.php>Teams</a></li> - join sample distributed computing project community
</ul>
<scheduler>http://maggie.ssl.berkeley.edu/barry-cgi/cgi</scheduler>
</body>
