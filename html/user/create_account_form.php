<?php
require_once('../inc/db.inc');
require_once('../inc/util.inc');
require_once('../inc/countries.inc');

db_init();
page_head('Create an account');

$config = get_config();
if (parse_bool($config, "disable_account_creation")) {
    echo "
        <h1>Account Creation is Disabled</h1>
        <p>
        Account creation is disabled for ".PROJECT." at the moment.
        Please try again later.
        </p>
    ";
    page_tail();
    exit();
}
echo "

    <p><b>Read the <a href=info.php>Rules and Policies</a>
    before creating an account.</b></p>

    <p>If you already received an account ID, do not submit this form.
    <a href=account_created.php>Activate your account</a> instead.</p>

    <form action=create_account_action.php method=post>
";
$teamid = $_GET['teamid'];
if ($teamid) {
    $team = lookup_team($teamid);
    $user = lookup_user_id($team->userid);
    if (!$user) {
        echo "No such user";
    } else {
        echo "<b>This account will belong the team
            <a href=show_team.php?teamid=$team->id>$team->name</a>
            and will have the project preferences of its founder.</b>
            <p>
        ";
        echo "
            <input type=hidden name=teamid value=$teamid>
        ";
    }
}
start_table();
row2(
    "Name<br><span class=description>Identifies you on our web site. Use your real name or a nickname.</span>",
    "<input name=new_name size=30>"
);
row2(
    "Email Address<br><span class=description>Must be a valid address of the form 'name@domain'.</span>",
    "<input name=new_email_addr size=50>"
);
row2_init(
    "Country <br><span class=description>Select the country you want to represent, if any.</span>",
    "<select name=country>"
);
print_country_select();
echo "</select></td></tr>\n";
row2(
    "Postal or ZIP Code <br><span class=description>Optional.</span>",
    "<input name=postal_code size=20>"
);
row2("",
    "<input type=submit value='Create Account'>"
);
end_table();
echo "
    </form>
";

page_tail();
?>

