<?php
require_once("../inc/db.inc");

db_init();

function create_category($orderID, $name, $is_helpdesk) {
    $q = "insert into category (orderID, lang, name, is_helpdesk) values ($orderID, 1, '$name', $is_helpdesk)";
    $result = mysql_query($q);
    if (!$result) {
        echo "can't create category\n";
        echo mysql_error();
        exit();
    }
    return mysql_insert_id();
}

function create_forum($category, $orderID, $title, $description) {
    $q = "insert into forum (category, orderID, title, description) values ($category, $orderID, '$title', '$description')";
    $result = mysql_query($q);
    if (!$result) {
        echo "can't create forum\n";
        echo mysql_error();
        exit();
    }
    return mysql_insert_id();
}

$catid = create_category(0, "", 0);
create_forum($catid, 0, "SETI@home Science", "Life in the universe, radio SETI, and SETI@home\'s search for ET");
create_forum($catid, 1, "Number crunching", "Credit, leaderboards, CPU performance");
create_forum($catid, 2, "Cafe SETI", "Trade stories with other SETI@home users");

$catid = create_category(0, "Platform-specific problems", 1);
create_forum($catid, 0, "Windows", "Installing and running BOINC on Windows");
create_forum($catid, 1, "Unix/Linux", "Installing and running BOINC on Unix and Linux");
create_forum($catid, 2, "Macintosh", "Installing and running BOINC on Mac OS/X");
$catid = create_category(1, "General issues", 1);
create_forum($catid, 3, "Getting started", "Creating your account");
create_forum($catid, 4, "Preferences", "Using preferences to fine-tune SETI@home");
create_forum($catid, 5, "Wish list", "What features would you like to see in BOINC and SETI@home");
?>
