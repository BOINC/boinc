<?php

function print_login_form() {
    page_head("Log in");
    echo "<br>\n";
    start_table_instr();
    echo "<tr><td>If you already have an account, you can login using the email address";
    echo " and password you used in creating your account. Otherwise you need to create";
    echo " a new account using a valid email address and password, both of which you will";
    echo " need in order to access your account in the future.</td></tr>\n";
    echo "</table>\n";
  
    printf(
        "<form method=post action=login_action.php>\n"
        ."<table>\n"
        ."<tr><td colspan=2>If you already have an account:</td></tr>\n"
        ."<tr>\n"
        ."    <td align=right>Your account email address</td>\n"
        ."    <td> <input name=existing_email type=text size=40></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td align=right>Your account password</td>\n"
        ."    <td> <input name=existing_password type=password></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td><br></td>\n"
        ."    <td><input type=submit name=old value=\"Log in\"></td>\n"
        ."</tr>\n"
        ."</table>\n"
        ."</form>\n"
        ."<table>\n"
        ."<tr><td colspan=2>If you don't have an account:</td></tr>\n"
        ."<tr><td><a href=create_account.php>Create a new account</a></td></tr>\n"
        ."</table>\n"
    );

}

function print_create_account_form() {
    printf(
        "<form method=post action=login_action.php>\n"
        ."<table>\n"
        ."<tr><td colspan=2>To create a new account:</td></tr>\n"
        ."<tr>\n"
        ."    <td align=right>Name or nickname</td>\n"
        ."    <td><input name=new_name size=30></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td align=right>Password</td>\n"
        ."    <td><input name=new_password type=password></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td align=right>Retype password</td>\n"
        ."    <td><input name=new_password2 type=password></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td align=right>Email</td>\n"
        ."    <td><input name=new_email_addr size=40></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td align=right>Country</td>\n"
        ."    <td><select name=country>\n"

    );
        print_country_select();
    printf(
        "    </select>\n"
        ."    </td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td align=right>Postal (ZIP) code</td>\n"
        ."    <td><input name=postal_code size=40></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td><br></td>\n"
        ."    <td><input type=submit name=new value=\"Create account\"></td>\n"
        ."</tr>\n".
        "</table>\n"
        ."</form>\n"
    );
}

?>
