<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");

    $project = db_init();
    if (strlen($HTTP_POST_VARS["old"])) {
        $query = sprintf(
            "select * from user where email_addr='%s'",
            $HTTP_POST_VARS["existing_email"]
        );
        $result = mysql_query($query);
        if ($result) {
            $user = mysql_fetch_object($result);
            mysql_free_result($result);
        }
        if (!$user) {
	    page_head("Logging in");
	    echo "There is no account with the email address you have entered.\n";
	    echo "Click the <b>Back</b> button to re-enter email address.\n"; 
	} else if ($user->web_password != $HTTP_POST_VARS["existing_password"]) {
	    page_head("Logging in");
            echo BADPASS; 
        } else {
            setcookie("auth", $user->authenticator, time()+100000000);
            page_head("User Page");   
            show_user_page($user, $project);
        }
    } else if (strlen($HTTP_POST_VARS["new"])) {
        $query = sprintf(
            "select * from user where email_addr='%s'",
            $HTTP_POST_VARS["new_email_addr"]
        );
        $result = mysql_query($query);
        if ($result) {
            $user = mysql_fetch_object($result);
            mysql_free_result($result);
        }
        if (strlen($HTTP_POST_VARS["new_email_addr"]) == 0) {
            page_head("Creating Account");
            printf(
                TABLE2."\n"
                ."<tr><td>You must enter an email address to create an account.\n"
                ."</td></tr>\n"
                ."</table>"
            );
        } else if (strlen($HTTP_POST_VARS["new_password"]) == 0) {
            page_head("Creating Account");
            printf(
                TABLE2."\n"
                ."<tr><td>You must enter a web password to create an account.\n"
                ."</td></tr>\n"
                ."</table>"
            );
        } else if ($user) {
            page_head("Creating Account");
            printf(
                TABLE2."\n"
                ."<tr><td>There's already an account with that email address. Click the <b>Back</b> button\n"
                ." on your browser to edit your information, or <a href=login.php>login </a>to your \n"
                .$project." account.</td></tr>\n"
                ."</table>\n"
            );
        } else {
            if ($HTTP_POST_VARS["new_password"] != $HTTP_POST_VARS["new_password2"]) {
                page_head("Creating Account");
                printf(
                    TABLE2."\n"
                    ."<tr><td>".DIFFPASS
                    ."</td></tr>\n"
                    ."</table>\n"
                );
            } else {
                $authenticator = random_string();
                $email_addr = $HTTP_POST_VARS["new_email_addr"];
                $query = sprintf(
                   "insert into user (create_time, email_addr, name, web_password, authenticator, country, postal_code) values(%d, '%s', '%s', '%s', '%s', '%s', %d)",
                    time(),
                    $email_addr,
                    $HTTP_POST_VARS["new_name"],
                    $HTTP_POST_VARS["new_password"],
                    $authenticator,
                    $HTTP_POST_VARS["country"],
                    $HTTP_POST_VARS["postal_code"]
                );
                $result = mysql_query($query);
                if ($result) {
                    setcookie("auth", $authenticator);
                    page_head("Creating Account");
                    printf(
                        TABLE2."\n"
                        ."<tr><td>Account has been created successfully. In order to run the client you will need a BOINC key. A key will be sent to \n"
                        ."the email address you provided, and you can simply copy and paste the key, which will be a string of letters and numbers, \n"
                        ."in the location indicated when you run the client.</td></tr>\n"
                        ."<tr><td><br><br></td></tr>\n"
                        ."<tr><td><a href=download.php>Download core client</a></td></tr>\n"
                        ."</table>\n"
                    );
                    mail($email_addr, "BOINC key", "Your BOINC key is " . $authenticator);
                } else {
                    page_head("Creating Account");
                    printf(
                        TABLE2."\n"
                        ."<tr><td>Couldn't create account. Please try again later.</td></tr>\n"
                        ."</table>\n"
                    );
                }
            }
        }
    }
    page_tail();
?>
