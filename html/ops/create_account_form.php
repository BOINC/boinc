<?php
// use this to create accounts while regular account creation is disabled

require_once("../inc/util_ops.inc");

echo "
    <h2>Create ".PROJECT." account</h2>
    <form action=create_account_action.php>
";
start_table();
row2("Name", "<input size=32 name=user_name>");
row2("Email address", "<input size=32 name=email_addr>");
row2("", "<input type=submit value=OK>");
echo "
    </form>
";

?>
