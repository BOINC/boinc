<?php

include_once("util.inc");

if (parse_config("<disable_account_creation/>")) {
    page_head("Account creation is disabled");
    echo "
        <h3>Account creation is disabled</h3>
        Sorry, this project has disabled the creation of new accounts.
        Please try again later.
    ";
    exit();
}

page_head("Create account");

echo "<h3>Create an account with ".PROJECT."</h3>
    <form method=post action=create_account_action.php>
    <p>
    <b>Read the <a href=info.php>rules and policies</a> before creating
    an account</b>
    <p>
";


start_table();
row2("<b>Name</b>
    <br><font size=-1>
    Identifies you on our web site.
    Use your real name or a nickname.
    </font>",
    "<input name=new_name size=30>"
);
row2("<b>Email address</b>
    <br><font size=-1>
    Must be a valid address of the form <i>name@domain</i>.
    </font>",
    "<input name=new_email_addr size=50>"
);

row2_init("<b>Country</b>
    <br><font size=-1>Select the country you want to represent, if any.</font>",
    "<select name=country>"
);
print_country_select();
echo "</select></td></tr>\n";

row2("<b>Postal or ZIP code</b>
    <br><font size=-1>Optional</font>",
    "<input name=postal_code size=20>"
);

row2("", "<input type=submit value=\"Create account\">");
end_table();
echo "</form>\n";

echo "If you already received an account key,
    <a href=account_created.php>click here</a>
    to activate your account
";

page_tail();

?>
