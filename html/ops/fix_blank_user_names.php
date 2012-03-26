<?php

// replace user names that are all whitespace with the user ID

require_once("../inc/boinc_db.inc");

$users = BoincUser::enum("trim(name)=''");
foreach ($users as $u) {
    $n = (string)($u->id);
    $u->update("name='$n'");
}
?>
