<?php

    require_once("db.inc");
    require_once("edit.inc");
    require_once("util.inc");

    db_init();
    $user = get_user_from_cookie();

    page_head("Change Password");
    if (!$user) {
        print_login_form();
        exit();
    }
    if ($HTTP_POST_VARS["my_pass"] == $user->web_password) {
        if ($HTTP_POST_VARS["new_pass"] != $HTTP_POST_VARS["new_pass2"]) {
            printf(
                TABLE2."\n"
                ."<tr><td>".DIFFPASS."</td></tr>\n"
                ."</table>"
            );
        } else {
            $query = sprintf("update user set web_password='%s' where id=%d", 
                            $HTTP_POST_VARS["new_pass"], $user->id);
            $result = mysql_query($query);
            if ($result) {
                printf(
                    TABLE2."\n"
                    ."<tr><td>Password changed successfully. Use your new password to\n"
                    ." <a href=login.php>login</a> to your account.</td></tr>\n"
                    ."</table>\n"
                );
            } else {
                printf(
                    TABLE2."\n"
                    ."<tr><td>Password was unable to be changed. Continue using your old \n"
                    ."password to <a href=login.php>login</a> to your account. You can try \n"
                    ."changing your password again later.</td></tr>\n"
                    ."</table>"
                );
            }
        }
    } else {
        printf(
               TABLE2."\n"
               ."<tr>".TD2.BADPASS."</td></tr>\n"
               ."</table>\n"
              ); 
    }
    page_tail();
?>
