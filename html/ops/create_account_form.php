<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
// use this to create accounts while regular account creation is disabled

echo "DEPRECATED\n";
exit();

require_once("../inc/util_ops.inc");

admin_page_head("Create Account");
echo "
    <form action=\"create_account_action.php\">
";
start_table();
row2("Name", "<input size=\"32\" name=\"user_name\">");
row2("Email address", "<input size=\"32\" name=\"email_addr\">");
row2("", "<input type=\"submit\" value=\"OK\">");
end_table();
echo "
    </form>
";

admin_page_tail();
?>
