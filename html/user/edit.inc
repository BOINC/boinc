<?php
   
function print_edit_user_info($user) {
    printf( 
	"<form method=post action=edit.php>\n"
  	.TABLE2."\n";
    	."<tr>".TD3.LG_FONT."<b>User Information:</b></font></td></tr>\n"
    );
    row3("<b>User name:   </b>", "<input name=my_name type=text size=30>", $user->name);
    row3("<b>Email address:   </b>", "<input name=my_email type=text size=50>", $user->email_addr);
    echo "<tr><td><b>Country:   </b></td><td><select name=my_country></td>\n";
    print_country_select();
    echo "</select>";
    row3("<b>Postal (ZIP) code:   </b>", "<input name=my_zip type=text size=20>", $user->postal_code);
    echo "<tr>".TD3."<br>/td></tr>\n";
    echo "<tr>".TD3."<input type=submit value=\"Edit\"></td></tr>\n"; 
    echo "</table>\n";
}

?>
