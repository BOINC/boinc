<?php
   
function print_edit_user_info($user) {
    printf( 
	"<form method=post action=edit_action.php>\n"
  	.TABLE2."\n"
	."<tr><td colspan=2><b>To edit your User Information, fill out the fields that you wish to change.\n"
        ." You do not need to enter anything into the fields that you wish to keep the same.</td></tr>\n"
	."<tr><td colspan=2><br></td></tr>\n"
	."<tr>".TD2.LG_FONT."<b>User name:</b></td></tr>\n"
    );
    row2("<input name=my_name type=text size=30>", $user->name);
    echo "<tr><td colspan=2><br></td></tr>\n";
    echo "<tr>".TD2.LG_FONT."<b>Email address:</b></td></tr>\n";
    row2("<input name=my_email type=text size=50>", $user->email_addr);	
    echo "<tr><td colspan=2><br></td></tr>\n";
    echo "<tr>".TD2.LG_FONT."<b>Country:</b></td></tr>\n";
    echo "<tr><td><select name=my_country>\n";
    print_country_select();
    echo "</select></td>\n";
    echo "<td>$user->country</td></tr>\n";
    echo "<tr><td colspan=2><br></td></tr>\n";
    echo "<tr>".TD2.LG_FONT."<b>Postal (ZIP) code:</b></td></tr>\n"; 
    row2("<input name=my_zip type=text size=20>", $user->postal_code);
    echo "</table>\n";
    echo "<br><br>&#160;&#160;&#160;&#160;&#160;&#160;&#160;<input type=submit value=\"Edit\">\n";
    echo "</form>";
}

function print_change_password($user) {
    printf(
	"<form method=post action=change_pass_action.php>\n"
	.TABLE2."\n"
    	. "<tr>".TD.LG_FONT."<b>Old Password:</b></td></tr>\n"
	."<tr><td><b>To change your password, please enter your old password below:</b></td></tr>\n"
     	."<tr><td><input name=my_pass type=password></td></tr>\n"
     	."<tr><td><br></td></tr>\n" 
     	."<tr>".TD.LG_FONT."<b>New Password:</b></td></tr>\n"
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
	printf(
                TABLE2."\n"
                ."<tr><td>There's already an account with that email address. Click the <b>Back</b> button\n"
            	." on your browser to edit your information, or <a href=login.php>login </a>to your \n"
            	.$project." account.</td></tr>\n"
		."<tr><td>Any other changes you've made, if any, were successfully updated.</td></tr>\n"
            	."</table>\n"
        );
    } else if ($e_ok == EMAIL_FAIL) {
	printf(
		TABLE2."\n"
		."<tr><td>Your email address failed to be updated. Click the <b>Back</b> button\n"
		." on your browser to edit your information, or try again later.</td></tr>\n"
		."<tr><td>Any other changes you've made, if any, were successfully updated.</td></tr>\n"
		."</table>\n"
	);
    } else if ($e_ok == EMAIL_UPDATED) {
	printf(
	  	TABLE2."\n"
		."<tr><td>Your information was successfully updated. A new, temporary password will be sent to the\n"
		." email address you provided. You must use this new password the next time you login.</td></tr>\n"
		."</table>\n"
	);
    } else {
	printf(
		TABLE2."\n"
		."<tr><td>Your information was successfully updated.</td></tr>\n"
		."</table>\n"
	);
    }
}

function print_update_fail($e_ok) {
    if ($e_ok == EMAIL_EXISTS) {
	printf(
                TABLE2."\n"
            	."<tr><td>There's already an account with that email address. Click the <b>Back</b> button\n"
            	." on your browser to edit your information, or <a href=login.php>login </a>to your \n"
            	.$project." account. </td></tr>\n"
		."<tr><td>Any other changes you've made, if any, failed to be updated. Please try again later.</td></tr>\n" 
            	."</table>\n"
        );
   } else if ($e_ok == EMAIL_FAIL) {
        printf(
                TABLE2."\n"
                ."<tr><td>Your information failed to be updated. Click the <b>Back</b> button\n"
                ." on your browser to edit your information, or try again later.</td></tr>\n"
                ."</table>\n"
        );
    } else if ($e_ok == EMAIL_UPDATED) {
        printf(
                TABLE2."\n"
                ."<tr><td>Your email address was successfully updated. A new, temporary password will be sent to the\n"
                ." email address you provided. You must use this new password the next time you login.</td></tr>\n"
                ."<tr><td>Any other changes you've made, if any, failed to be updated. Please try again later.</td></tr>\n"
                ."</table>\n"
        );
    } else {
	printf(
		TABLE2."\n"
		."<tr><td>Your information failed to be updated. Please try again later.</td></tr>\n"
		."</table>\n"
	);
    }
}

?>
