<?php

function getSingleQuery($query)
{
    $result = mysql_query($query);
    if (!$result) return;
    $cnt = mysql_fetch_row($result);
    if (!$cnt) return;
    mysql_free_result($result);
    return $cnt[0];
}

    require_once("../inc/util.inc");
    require_once("../inc/db.inc");
    //require_once("../inc/trickle.inc");
    require_once("../inc/wap.inc");
 
    // show the home page of app user from envvar

    $valid = $_GET['id'];
    if (!$valid || $valid!="whatever-validation-key-you-want") {
        echo "User id (t.php?id=###) missing!";
        exit(); // can't do much without a userid!
    }

    db_init();

   wap_begin();

    // keep a 'running tab' in wapstr in case exceeds 1K WAP limit

    $wapstr = PROJECT . "<br/>Status Info on<br/>" . wap_timestamp() . "<br/><br/>";

    $wapstr .= "#Users: " . getSingleQuery("select count(*) from user") . "<br/>";
    $wapstr .= "#Hosts: " . getSingleQuery("select count(*) from host") . "<br/>";
    $wapstr .= "#ModYr: " . sprintf("%ld", getSingleQuery("select sum(total_credit)/(.007*17280.0) from host")) . "<br/>";
    $wapstr .= "#Cobbl: " . sprintf("%ld", getSingleQuery("select sum(total_credit) from host")) . "<br/>";
    // I consider a host active if it's trickled in the last week
    //$wapstr .= "#Activ: " . getSingleQuery("select count(distinct hostid) from cpdnexpt.trickle "
    //   . "where trickledate>=" . sprintf("%d", mktime() - (3600*24*7))) . "<br/>";

   // finally get last 5 trickles for everyone
   //$wapstr .= show_trickles("a", 0, 5, 1);

   // limit wap output to 1KB
   if (strlen($wapstr)>1024)
       echo substr($wapstr,0,1024);
   else
       echo $wapstr;

   wap_end();

?>
