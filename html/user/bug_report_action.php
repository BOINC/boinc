<?php

include_once("db.inc");
include_once("util.inc");

$authenticator = init_session();
db_init();

$user = get_user_from_auth($authenticator);
if ($user == NULL) {
    print_login_form();
    exit();
}

$f = fopen("bug_reports.xml", "a");
$x = sprintf("<bug>
    <userid>$user->id</userid>
    <platform>%s</platform>
    <problem>
%s
    </problem>
</bug>
",
    $_POST["platform"],
    $_POST["problem"]
);
fputs($f, $x);
fclose($f);

page_head("Problem report recorded", $user);
echo "
    Your problem report has been recorded.
    We apologize for any inconvience you may have experienced.
";
page_tail();
?>
