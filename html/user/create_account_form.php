<?php
require_once('../inc/db.inc');
require_once('../inc/util.inc');
require_once('../inc/countries.inc');
require_once('../inc/translation.inc');

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
echo "

    <p><b>"; printf(tr(CREATE_AC_READ_RULES), "<a href=info.php>".tr(RULES_TITLE)."</a>");echo "</b></p>

    <p>"; printf(tr(CREATE_AC_ALREADY_GOT), "<a href=account_created.php>".tr(AC_CREATED_TITLE)."</a>"); echo "
    </p>

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
	printf(tr(CREATE_AC_TEAM), "<a href=\"show_team.php?teamid=$team->id\">$team->name</a>");
	echo "</b>
            <p>
        ";
        echo "
            <input type=hidden name=teamid value=$teamid>
        ";
    }
}
start_table();
row2(
    tr(CREATE_AC_NAME)."<br><span class=description>".tr(CREATE_AC_NAME_DESC)."</span>",
    "<input name=new_name size=30>"
);
row2(
    tr(CREATE_AC_EMAIL)."<br><span class=description>".tr(CREATE_AC_EMAIL_DESC)."</span>",
    "<input name=new_email_addr size=50>"
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
    "<input type=submit value='".tr(CREATE_AC_CREATE)."'>"
);
end_table();
echo "
    </form>
";

page_tail();
?>

