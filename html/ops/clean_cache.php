#! /usr/bin/env php
<?php
require_once("../inc/cache.inc");

set_time_limit(0);

function cache_check_diskspace2(){
   $too_old = 86400;
   while (1) {
       $f = disk_free_space("../cache");
       $u = disk_usage("../cache");
       echo "free: $f used: $u\n";
       if ($f > MIN_FREE_SPACE && $u < MAX_CACHE_USAGE) {
           break;
       }
       clean_cache($too_old, "../cache");
       $too_old/=2;
   }
}

cache_check_diskspace2();
?>
