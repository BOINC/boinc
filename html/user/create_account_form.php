<?php
require_once('../inc/db.inc');
require_once('../inc/util.inc');
require_once('../inc/countries.inc');
require_once('../inc/translation.inc');

// Web-based interface for account creation.
// This isn't needed for people who use the version 5 Manager's
// "attach project wizard".
// But (for the time being at least)
// it's needed for pre-version 5 clients,
// and clients that don't use the account manager

db_init();
page_head(tr(CREATE_AC_TITLE));

$config = get_config();
if (parse_bool($config, "disable_account_creation")) {
    echo "
        <h1>".tr(CREATE_AC_DISABLED)."</h1>
        <p>".tr(CREATE_AC_DISABLED_TEXT)."
        </p>
    ";
    page_tail();
    exit();
}
echo "<p>
    <b>".tr(CREATE_AC_USE_CLIENT)."</b>
";
echo "
    <p>
    <form action=create_account_action.php method=post>
";
$teamid = get_int("teamid", true);
if ($teamid) {
    $team = lookup_team($teamid);
    $user = lookup_user_id($team->userid);
    if (!$user) {
        echo "No such user";
    } else {
        echo "<b>";
        printf(tr(CREATE_AC_TEAM), "<a href=\"team_display.php?teamid=$team->id\">$team->name</a>");
        echo "</b> <p> ";
        echo "
            <input type=hidden name=teamid value=$teamid>
        ";
    }
}
start_table();

// Using invitation codes to restrict access?
//
if(defined('INVITE_CODES')) {
     row2(
         tr(AC_INVITE_CODE)."<br><span class=description>".tr(AC_INVITE_CODE_DESC)."</span",
         "<input name=invite_code size=30>"
     );
} 

row2(
    tr(CREATE_AC_NAME)."<br><span class=description>".tr(CREATE_AC_NAME_DESC)."</span>",
    "<input name=new_name size=30>"
);
row2(
    tr(CREATE_AC_EMAIL)."<br><span class=description>".tr(CREATE_AC_EMAIL_DESC)."</span>",
    "<input name=new_email_addr size=50>"
);
$min_passwd_length = parse_element($config, "<min_passwd_length>");
if (!$min_passwd_length) {
    $min_passwd_length = 6;
}

row2(
    tr(CREATE_AC_PASSWORD)
    ."<br><span class=description>"
    .sprintf(tr(CREATE_AC_PASSWORD_DESC), $min_passwd_length)
    ." </span>",
    "<input type=password name=passwd>"
);
row2(tr(CREATE_AC_CONFIRM_PASSWORD), "<input type=password name=passwd2>");
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
    "<input type=submit value='".tr(CREATE_AC_CREATE)."'>"
);
end_table();
echo "
    </form>
";

page_tail();
?>

