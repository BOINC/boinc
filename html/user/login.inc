<?php

function print_login_form() {
    printf(
        "<form method=post action=login_action.php>\n"
        .TABLE2."\n"
        ."<tr><td colspan=2><b>Enter the email address and password of your account to login:</b></td></tr>\n"
        ."<tr><td><br></td></tr>\n"
        ."<tr>\n"
        ."    <td align=right>Email Address</td>\n"
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
    );
}

function print_create_account_form() {
    printf(
        "<form method=post action=login_action.php>\n"
        .TABLE2."\n"
        ."<tr><td><b>To create a new account, please fill out the following information:</b></td></tr>\n"
        ."<tr><td><br></td></tr>\n"
        ."<tr>\n"
        ."    <td><b>Name or nickname:</b>".SM_FONT."  this is the name that will be seen by other users on the web.</font></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td><input name=new_name size=30></td>\n"
        ."</tr>\n"
        ."<tr><td><br></td></tr>\n"
        ."<tr>\n"
        ."    <td><b>Email address:</b>".SM_FONT."  this is what you will be using to login to your account. The address you enter\n"
        ."must be valid in order for you to create an account; a unique BOINC key will be sent to it to make sure it is valid.</font></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td><input name=new_email_addr size=50></td>\n"
        ."</tr>\n"
        ."<tr><td><br></td></tr>\n"
        ."<tr>\n"
        ."    <td><b>Password:</b>".SM_FONT."  remember this password - you may need it to access your account in the future.</font></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td><input name=new_password type=password></td>\n"
        ."</tr>\n"
        ."<tr><td><br></td></tr>\n"
        ."<tr>\n"
        ."    <td><b>Retype password to confirm:</b></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td><input name=new_password2 type=password></td>\n"
        ."</tr>\n"
        ."<tr><td><br></td></tr>\n"
        ."<tr>\n"
        ."    <td><b>Country:</b>".SM_FONT."  select from the list below the country you wish to represent.</font></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td><select name=country>\n"

    );
        print_country_select();
    printf(
        "    </select>\n"
        ."    </td>\n"
        ."</tr>\n"
        ."<tr><td><br></td></tr>\n"
        ."<tr>\n"
        ."    <td><b>Postal (ZIP) code:</b></td>\n"
        ."</tr>\n"
        ."<tr>\n"
        ."    <td><input name=postal_code size=20></td>\n"
        ."</tr>\n"
        ."<tr><td><br></td></tr>\n"
        ."<tr>\n"
        ."    <td><input type=submit name=new value=\"Create account\"></td>\n"
        ."</tr>\n".
        "</table>\n"
        ."</form>\n"
    );
}

?>
