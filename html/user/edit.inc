<?php
   
function print_edit_user_info($user) {
    printf( 
	"<form method=post action=edit_action.php>\n"
  	.TABLE2."\n"
    	."<tr>".TD3.LG_FONT."<b>User Information:</b></font></td></tr>\n"
    );
    row3("<b>User name:   </b>", "<input name=my_name type=text size=30>", $user->name);
    row3("<b>Email address:   </b>", "<input name=my_email type=text size=50>", $user->email_addr);
    echo "<tr><td><b>Country:   </b></td><td><select name=my_country>\n";
    print_country_select();
    echo "</select></td>\n";
    echo "<td>$user->country</td></tr>\n"; 
    row3("<b>Postal (ZIP) code:   </b>", "<input name=my_zip type=text size=20>", $user->postal_code);
    echo "</table>\n";
    echo "<br><br>&#160;&#160;&#160;&#160;&#160;&#160;&#160;<input type=submit value=\"Edit\">\n";
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
    else if ($e_ok == EMAIL_FAIL) {
	printf(
		TABLE2."\n"
		."<tr><td>Your email address failed to be updated. Click the <b>Back</b> button\n"
		." on your browser to edit your information, or try again later.</td></tr>\n"
		."<tr><td>Any other changes you've made, if any, were successfully update.</td></tr>\n"
		."</table>\n"
	);
    else {
	printf(
	  	TABLE2."\n"
		."<tr><td>Your email address was successfully updated.
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
}

?>
