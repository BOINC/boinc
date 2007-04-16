<?
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

$catid = create_category(0, "General", 0);
create_forum($catid, 0, "General", "General discussion");

$catid = create_category(0, "General", 1);
create_forum($catid, 0, "General", "General questions");

?>
