<?php

function print_login_form() {
    printf(
        "<form method=post action=login_action.php>\n"
        ."<table>\n"
        ."<tr><td colspan=2>If you already have an account:</td></tr>\n"
        ."<tr>\n"
        ."    <td align=right>Email</td>\n"
        ."    <td> <input name=existing_email type=text size=40></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td align=right>Password</td>\n"
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
        ."    <td align=right>Password confirm</td>\n"
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
