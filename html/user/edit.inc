<?php
   
function print_change_password($user) {
    printf(
	"<form method=post action=change_pass_action.php>\n"
	.TABLE2."\n"
    	. "<tr>".TD.LG_FONT."<b>Old Password:</b></font></td></tr>\n"
	."<tr><td><b>To change your password, please enter your old password below:</b></td></tr>\n"
     	."<tr><td><input name=my_pass type=password></td></tr>\n"
     	."<tr><td><br></td></tr>\n" 
     	."<tr>".TD.LG_FONT."<b>New Password:</b></font></td></tr>\n"
	."<tr><td><b>Next, enter your new password:</b></td></tr>\n"
     	."<tr><td><input name=new_pass type=password></td></tr>\n"
    	."<tr><td><b>Retype password to confirm:</td></tr>"
	."<tr><td></b><input name=new_pass2 type=password></td></tr>\n"
     	."</table>\n"
     	."<br><br>&#160;&#160;&#160;&#160;&#160;&#160;&#160;<input type=submit value=\"Change Password\">\n"
     	."</form>\n"
    );
}


function print_update_ok($e_ok) {
    if ($e_ok == EMAIL_EXISTS) {
	echo TABLE2."\n
            <tr><td>There's already an account with that email address.
            Click the <b>Back</b> button\n
            in your browser to edit your information,
            or <a href=login_form.php>login </a>to your \n
            ".$project." account.</td></tr>\n
            <tr><td>Any other changes you've made,
            if any, were successfully updated.</td></tr>\n
            </table>\n";
    } else if ($e_ok == EMAIL_FAIL) {
	echo TABLE2."\n
            <tr><td>Your email address failed to be updated.
            Click the <b>Back</b> button\n
            in your browser to edit your information,
            or try again later.</td></tr>\n
            <tr><td>Any other changes you've made,
            if any, were successfully updated.</td></tr>\n
            </table>\n";
    } else if ($e_ok == EMAIL_UPDATED) {
	echo TABLE2."\n
            <tr><td>Your information was successfully updated.
            A new, temporary password will be sent to the\n
             email address you provided.
             You must use this new password the next time you login.
             </td></tr>\n</table>\n";
    } else {
	echo TABLE2."\n
            <tr><td>Your information was successfully updated.</td></tr>\n
            </table>\n";
    }
}

function print_update_fail($e_ok) {
    if ($e_ok == EMAIL_EXISTS) {
	echo TABLE2."\n
            <tr><td>There's already an account with that email address.
            Click the <b>Back</b> button in your browser
            to edit your information,
            or <a href=login_form.php>login </a>to your \n
            ".$project." account. </td></tr>\n
            <tr><td>Any other changes you've made, if any,
            failed to be updated. Please try again later.</td></tr>\n
            </table>\n";
   } else if ($e_ok == EMAIL_FAIL) {
        echo TABLE2."\n
            <tr><td>Your information failed to be updated.
            Click the <b>Back</b> button\n
            in your browser to edit your information,
            or try again later.</td></tr>\n
            </table>\n";
    } else if ($e_ok == EMAIL_UPDATED) {
        echo TABLE2."\n
            <tr><td>Your email address was successfully updated.
            A new, temporary password will be sent to the\n
            email address you provided.
            You must use this new password the next time you login.</td></tr>\n
            <tr><td>Any other changes you've made, if any,
            failed to be updated. Please try again later.</td></tr>\n
            </table>\n";
    } else {
	echo TABLE2."\n
            <tr><td>Your information failed to be updated.
            Please try again later.</td></tr>\n
            </table>\n";
    }
}

?>
