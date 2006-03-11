<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

function format_float($x) {
    if ($x<1.e-5) {
        return '0';
    } else if ($x<0.01) {
        return sprintf("%0.5f", $x);
    } else if ($x<0.1) {
        return sprintf("%0.4f", $x);
    } else if ($x<1) {
        return sprintf("%0.3f", $x);
    } else if ($x<10) {
        return sprintf("%0.2f", $x);
    } else if ($x<100) {
        return sprintf("%0.1f", $x);
    } else if ($x<1000) {
        return sprintf("%0.0f", $x);
    } else {
        return number_format($x);
    }
    return '';
}

function show_user($user, $which) {
    $rac=format_float($user->expavg_credit);
    $tot=format_float($user->total_credit);
    echo 
"  <tr>
    <td align=\"center\">$which</td>
    <td align=\"center\">".user_links($user)."</td>
    <td align=\"center\">".date_str($user->create_time)."</td>
    <td align=\"center\">".$user->country."</td>
    <td align=\"center\">$tot</td>
    <td align=\"center\">$rac</td>
  </tr>\n";
}

function print_table_header($urls, $nextd) {
    echo
"<table align=\"center\" cellpadding=\"2\" border=\"1\" width=\"100%\">
  <tr>
    <th align=\"center\">Position</th>
    <th align=\"center\"><a href=user_search_action.php?search_string=$urls&offset=0&descending=$nextd&order=name>User name</a></th>
    <th align=\"center\"><a href=user_search_action.php?search_string=$urls&offset=0&descending=$nextd&order=create_time>Joined project</a></th>
    <th align=\"center\"><a href=user_search_action.php?search_string=$urls&offset=0&descending=$nextd&order=country>Country</a></th>
    <th align=\"center\"><a href=user_search_action.php?search_string=$urls&offset=0&descending=$nextd&order=total_credit>Total credit</a></th>
    <th align=\"center\"><a href=user_search_action.php?search_string=$urls&offset=0&descending=$nextd&order=expavg_credit>Recent credit</a></th>
  </tr>\n";
}

/* if needed to help prevent database stress, sleep! */
/* sleep(2); */

db_init();

$default_sort = 'id';
$allowed_order = array('id', 'name', 'create_time','country', 'total_credit', 'expavg_credit');
$nice_names    = array('', 'sorted by name', 'sorted by date joined', 'sorted by country', 'sorted by total credit', 'sorted by recent average credit');

if (!isset ($_GET['order']) || !in_array ($_GET['order'], $allowed_order)) {
    $order = $default_sort;
    $nice_name='';
} else {
    $order = $_GET['order'];
    $nice_name = $nice_names[array_search($order, $allowed_order)];
}

$search_string = get_str('search_string');

if (isset($_GET['offset'])) $offset = $_GET['offset'];
if (!is_numeric($offset) || $offset<0) $offset=0;

if (isset($_GET['descending']) && $_GET['descending']==1) {
    $upordown='desc';
    $descending=1;
    $nextd=0;
} else {
    $upordown='asc';
    $descending=0;
    $nextd=1;
}

$count = 100;

page_head("Search results");

if (strlen($search_string)>=3) {
    $urls = urlencode($search_string);
    $s = escape_pattern($search_string);
    $q = "select * from user where name like '$s%' order by $order $upordown limit $offset,$count";
    $result = mysql_query($q);
    
    $n=0;
    while ($user = mysql_fetch_object($result)) {
        if ($n==0) {
            echo "<h2>User names starting with '$search_string' $nice_name</h2>\n";
            print_table_header($urls, $nextd);
        }
        show_user($user, $n+$offset+1);
        $n++;
    }
    echo "</table>\n";
    mysql_free_result($result);
    if (!$n) {
        echo "<h2>No user names found starting with '$search_string'</h2>\n";
    }
    
    echo "<br><br>\n";
    echo "<h3>";
    
    if ($offset>=$count) {
        $prev= $offset-$count;
        echo "<a href=user_search_action.php?search_string=$urls&offset=$prev&descending=$descending&order=$order>Previous $count</a>&nbsp;&nbsp;&nbsp;&nbsp;\n";
    }
    if ($n==$count) {
        $next= $offset+$count;
        echo "<a href=user_search_action.php?search_string=$urls&offset=$next&descending=$descending&order=$order>Next $count</a>\n";
    }
    echo "</h3>";

} else {
    echo "<h2>Search string must be at least three characters long!</h2>\n";
}
echo "<br><h3><a href=profile_menu.php>Return to profile zone</a></h3><br>\n";

page_tail();
?>
