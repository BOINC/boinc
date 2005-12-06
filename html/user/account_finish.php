<?php

// Users are taken here after creating an account via the Wizard.
// They've already entered an email address and password.
// Now get a name, country, and zip code

require_once('../inc/db.inc');
require_once('../inc/util.inc');
require_once('../inc/countries.inc');
require_once('../inc/translation.inc');

db_init();
$auth = process_user_text(get_str("auth"));
$user = lookup_user_auth($auth);
if (!$user) {
    error_page("no such account");
}
page_head("Finish account setup");

echo "
    <form action=account_finish_action.php method=post>
";
start_table();
row2(
    tr(CREATE_AC_NAME)."<br><span class=description>".tr(CREATE_AC_NAME_DESC)."</span>",
    "<input name=name size=30 value=$user->name>"
);
row2_init(
    tr(CREATE_AC_COUNTRY)."<br><span class=description>".tr(CREATE_AC_COUNTRY_DESC)."</span>",
    "<select name=country>"
);
print_country_select();
echo "</select></td></tr>\n";
row2(
    tr(CREATE_AC_ZIP)."<br><span class=description>".tr(OPTIONAL).".</span>",
    "<input name=postal_code size=20>"
);
row2("",
    "<input type=submit value=OK>"
);
end_table();
echo "
    <input type=hidden name=auth value=$auth>
    </form>
";

page_tail();
