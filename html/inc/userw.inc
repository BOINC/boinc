<?php

function show_credit($user) {
    $retstr = "<br/>User TotCred: " . format_credit($user->total_credit) . "<br/>";
    $retstr .= "User AvgCred: " . format_credit($user->expavg_credit) . "<br/>";
    /*
    if ($user->seti_nresults) {
        row2("SETI@home classic workunits", number_format($user->seti_nresults));
    }
    if ($user->seti_total_cpu) {
        $x = number_format($user->seti_total_cpu/3600)." hours";
        row2("SETI@home classic CPU time", $x);
    }
    */
    return $retstr;
}

function show_user_wap($user)
{

   wap_begin();
 
   if (!$user) {
      echo "<br/>User not found!<br/>";
      wap_end();
      return;
   }

    // keep a 'running tab' in wapstr in case exceeds 1K WAP limit

    $wapstr = PROJECT . "<br/>Account Data<br/>for $user->name<br/>Time: " . wap_timestamp();
 
    $wapstr .= show_credit($user);

    if ($user->teamid) {
        $result = mysql_query("select name, total_credit, expavg_credit from team where id = $user->teamid");
        $team = mysql_fetch_object($result);

        $wapstr .= "<br/>Team: $team->name<br/>";
        $wapstr .= "Team TotCred: " . format_credit($team->total_credit) . "<br/>";
        $wapstr .= "Team AvgCred: " . format_credit($team->expavg_credit) . "<br/>";

        mysql_free_result($result);

    } else {
        $wapstr .= "<br/>Team: None<br/>";
    }

   // finally get last 5 trickles for user
   //$wapstr .= show_trickles("u", $user->id, 5, 1);

   // don't want to send more than 1KB probably?
   if (strlen($wapstr)>1024)
       echo substr($wapstr,0,1024);
   else
       echo $wapstr;

   wap_end();
}

?>
