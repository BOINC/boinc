<?php
require_once("../inc/util.inc");
require_once("../inc/db.inc");
require_once("../inc/wap.inc");
require_once("../inc/cache.inc");

function show_credit_wap($user) {
    $retstr = "<br/>User TotCred: " . format_credit($user->total_credit) . "<br/>";
    $retstr .= "User AvgCred: " . format_credit($user->expavg_credit) . "<br/>";
    return $retstr;
}

function show_user_wap($user) {
   wap_begin();
   if (!$user) {
      echo "<br/>User not found!<br/>";
      wap_end();
      return;
   }

    // keep a 'running tab' in wapstr in case exceeds 1K WAP limit

    $wapstr = PROJECT . "<br/>Account Data<br/>for $user->name<br/>Time: " . wap_timestamp();
    $wapstr .= show_credit_wap($user);
    if ($user->teamid) {
        $team = BoincTeam::lookup_id($user->teamid);
        $wapstr .= "<br/>Team: $team->name<br/>";
        $wapstr .= "Team TotCred: " . format_credit($team->total_credit) . "<br/>";
        $wapstr .= "Team AvgCred: " . format_credit($team->expavg_credit) . "<br/>";

    } else {
        $wapstr .= "<br/>Team: None<br/>";
    }

   // don't want to send more than 1KB probably?
   if (strlen($wapstr)>1024) {
       echo substr($wapstr,0,1024);
   } else {
       echo $wapstr;
   }

   wap_end();
}

$userid = get_int('id');

$cache_args = "userid=".$userid;
start_cache(USER_PAGE_TTL, $cache_args);

$user = BoincUser::lookup_id($userid);
if (!$user) {
    error_page("No such user");
}
show_user_wap($user);

end_cache(USER_PAGE_TTL, $cache_args);
?>
