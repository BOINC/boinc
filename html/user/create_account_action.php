<?php

include_once("util.inc");

function show_error($str) {
    page_head("Create account: error");
    echo "$str<br>\n";
    echo mysql_error();
    echo "<p>Click your browser's <b>Back</b> button to try again.\n<p>\n";
    page_tail();
    exit();
}

    if (parse_config("<disable_account_creation/>")) {
        page_head("Account creation is disabled");
        echo "
            <h3>Account creation is disabled</h3>
            Sorry, this project has disabled the creation of new accounts.
            Please try again later.
        ";
        exit();
    }

    init_session();
    db_init();

    $new_name = $HTTP_POST_VARS["new_name"];
    if (strlen($new_name)==0) {
        show_error("You must supply a name for your account");
    }

    $new_email_addr = $HTTP_POST_VARS["new_email_addr"];
    if (!is_valid_email_addr($new_email_addr)) {
        show_error("Invalid email address:
            you must enter a valid address of the form
            name@domain"
        );
    }
    $query = "select * from user where email_addr='$new_email_addr'";
    $result = mysql_query($query);
    if ($result) {
        $user = mysql_fetch_object($result);
        mysql_free_result($result);
        if ($user) {
            show_error("There's already an account with that email address");
        }
    }

    $authenticator = random_string();
    $munged_email_addr = munge_email_addr($new_email_addr, $authenticator);
    
    if (!empty($_POST['mirror']) && is_int($_POST['mirror'])) {
    	$sql = "SELECT project_prefs, teamid FROM user WHERE id = ".$_POST['mirror']." LIMIT 1";
    	$result = mysql_query($sql);
    	if ($result)
	    	$myrow = mysql_fetch_array($result);
	}
	$query = 'INSERT INTO user SET '
	        .' create_time = UNIX_TIMESTAMP(),'
	        ." email_addr = '$munged_email_addr',"
	        ." name = '$new_name',"'
	        ." authenticator = '$authenticator',"
	        ." country = '$_POST[country]',"
	        ." postal_code = '$_POST[postal_code]',"
	        ." total_credit = 0,"
	        ." expavg_credit = 0,"
	        ." expavg_time = 0,"
	        .(!empty($myrow['project_prefs']))?" project_prefs = '$myrow[project_prefs]',":""
	        ." teamid = '".(!empty($myrow['teamid']))?$myrow['teamid']:'0'."',"
	        ." venue = 'home',"
	        ." url = '',"
	        ." send_email = 1,"
	        ." show_hosts = 1";
    $result = mysql_query($query);
    if (!$result) {
        show_error("Couldn't create account");
    }

    // In success case, redirect to a fixed page so that user can
    // return to it without getting "Repost form data" stuff

    send_auth_email($new_email_addr, $authenticator);
    Header("Location: account_created.php?email_addr=$new_email_addr");
