<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// show top users or teams, ordered by per-app credit
//
// URL args:
// is_team: if nonzero, show teams
// appid: ID of app for sorting; default is first app returned by enum
// is_total: if nonzero, sort by total credit

require_once("../inc/util.inc");
require_once("../inc/team.inc");

check_get_args(array("is_team", "appid", "is_total", "added_offset", "up", "offset"));

define ('ITEM_LIMIT', 10000);

// return a column title (Average or Total),
// hyperlinked if this is not the current sort column
//
function col_title($is_team, $app, $appid, $is_total, $i) {
    $x = $i?"Total":"Average";
    if ($app->id == $appid && ($is_total?$i:!$i)) {
        return $x;
    } else {
        return "<a href=per_app_list.php?appid=$app->id&is_team=$is_team&is_total=$i>$x</a>";
    }
}

// print a row of app names,
// under each of which are columns for Average and Total
//
function show_header($is_team, $apps, $appid, $is_total) {
    echo "<tr><th colspan=2>&nbsp;</th>";
    foreach ($apps as $app) {
        echo "<th colspan=2>$app->name</th>\n";
    }
    echo "</tr>";

    echo "<tr>";
    echo "<th>Rank</th><th>Name</th>\n";
    foreach ($apps as $app) {
        for ($i=0; $i<2; $i++) {
            $x = col_title($is_team, $app, $appid, $is_total, $i);
            echo "<th>$x</th>\n";
        }
    }

    echo "</tr>";

}

// show a user or team, with their credit for each app
//
function show_row($item, $x) {
    global $i;
    global $apps;
    echo "<tr>";
    echo "<td>$i</td>";
   
    echo $item[0][$x];
    $y = 1;
    foreach ($apps as $app) {
         echo $item[$y][$x];
         $y++;
    }
    echo "</tr>";
    
}

function choose_teams($x, $appid, $used_offset, $up) {
     global $store;
     global $items_per_page;
     $fetched_items = $items_per_page;
     $check = 0;
     $fetch = 1;
    
     $store[1][0] = 0;
 
     //Check whether every one of the credit_teams corresponds to a valid team in the teams table
     //and fetch accordingly bigger chunks of the credit_team table until we have the chosen size
     //
     if (!$up) {
         while ($check != $fetch) {
            $data =  BoincCreditTeam::enum("appid=$appid order by $x desc limit $used_offset,$fetched_items");
            $check = sizeof($data) - ($fetched_items - $items_per_page);
            $fetch = $check + ($fetched_items - $items_per_page);
            foreach ($data as $item) {
                     $team = BoincTeam::lookup_id($item->teamid);
                     if (!is_object($team)) {
                       $fetch--;
                     }
            }
            if ($check != $fetch) {
                $fetched_items++;
            }
         }
         //How many teams did we get
         //
         $store[0][0] = $fetch;
         
         //How many more than the predefined items_per_page did we fetch
         //
         $store[1][0] = $fetched_items - $items_per_page;
     
     }   else {

         //Do the opposite. Fetch chunks from the credit_team table but from an earlier point 
         //every time we find a non valid team entry. 
         //
         while ($check != $fetch) {
               if ($used_offset < 0) $used_offset = 0;
               $data =  BoincCreditTeam::enum("appid=$appid order by $x desc limit $used_offset,$fetched_items");
               $check = sizeof($data) - ($fetched_items - $items_per_page);
               $fetch = $check + ($fetched_items - $items_per_page);
               foreach ($data as $item) {
                      $team = BoincTeam::lookup_id($item->teamid);
                      if (!is_object($team)) {
                        $fetch--;
                      }
             }
             if ($check != $fetch) {
                 $fetched_items++;
                 $used_offset--;
             }
        } 
        $store[0][0] = $fetch;
        $store[1][0] = $fetched_items - $items_per_page;
        }

       return $data;

}

function choose_users($x, $appid, $used_offset, $up) {
     global $store;
     global $items_per_page;
     $fetched_items = $items_per_page;
     $check = 0;
     $fetch = 1;
  
      //Do the same for the credit_user and user tables
      //
     if (!$up) {
      while ($check != $fetch) {
             $data = BoincCreditUser::enum("appid=$appid order by $x desc limit $used_offset,$fetched_items");
             $check = sizeof($data) - ($fetched_items - $items_per_page);
             $fetch = $check + ($fetched_items - $items_per_page);
             foreach ($data as $item) {
                      $user = BoincUser::lookup_id($item->userid);
                      if (!is_object($user)) {
                          $fetch--;
                      }
             }
             if ($check != $fetch) {
                   $fetched_items++;
             }
      }
        $store[0][0] = $fetch;
        $store[1][0] = $fetched_items - $items_per_page;
     } else { 
   
       while ($check != $fetch) {
              if ($used_offset < 0) $used_offset = 0;
              $data =  BoincCreditUser::enum("appid=$appid order by $x desc limit $used_offset,$fetched_items");
              $check = sizeof($data) - ($fetched_items - $items_per_page);
              $fetch = $check + ($fetched_items - $items_per_page);
              foreach ($data as $item) {
                       $user = BoincUser::lookup_id($item->userid);
                       if (!is_object($user)) {
                         $fetch--;
                       }
              }
              if ($check != $fetch) {
                  $fetched_items++;
                  $used_offset--;
              }
        }
           $store[0][0] = $fetch;
           $store[1][0] = $fetched_items - $items_per_page;
       }
        return $data;
}

function retrieve_credit_team($data) {
     global $apps;
     global $store;
     $x = 1;
     $c = 1;
   
     //Store the sign of each team in the first column of an array
     // 
     foreach ($data as $item) {
                $team = BoincTeam::lookup_id($item->teamid);
                if (is_object($team)) {
                   $sign = "<td>".team_links($team)."</td>";
                   $store[0][$c] = $sign;
                   $c++;
                }
     }

     //Store the expavg and total credits of each team and for each app in 
     //their corresponding row of the array
     //
     foreach ($apps as $app) {
     $y=1;

        foreach ($data as $item) {
               $team = BoincTeam::lookup_id($item->teamid);
                if (is_object($team)) {
                    $item = BoincCreditTeam::lookup("teamid=$item->teamid and appid=$app->id");
                
                if (is_object($item)) {
                    $store[$x][$y] = "<td>".format_credit($item->expavg) ."</td><td>". format_credit_large($item->total) ."</td>";
                } else {
                    $store[$x][$y] = "<td>"."No Credit"."</td><td>"."No Credit"."</td>";
                }
                $y++;
               }
        }

     $x++;
    }
}

function retrieve_credit_user($data) {
     global $apps;
     global $store;
     $x = 1;
     $c = 1;

     //Same as above 
     //
     foreach ($data as $item) {
              $user = BoincUser::lookup_id($item->userid);
              if (is_object($user)) {
                  $sign = "<td>".user_links($user, BADGE_HEIGHT_MEDIUM)."</td>";
                  $store[0][$c] = $sign;
                  $c++;
              }
     }

     foreach ($apps as $app) {
     $y=1;

              foreach ($data as $item) {
                       $user = BoincUser::lookup_id($item->userid);
                       if (is_object($user)) {
                          $item = BoincCreditUser::lookup("userid=$item->userid and appid=$app->id");

                          if (is_object($item)) {
                              $store[$x][$y] = "<td>".format_credit($item->expavg) ."</td><td>". format_credit_large($item->total) ."</td>"    ;
                          } else {
                              $store[$x][$y] = "<td>"."No Credit"."</td><td>"."No Credit"."</td>";
                          }
                          $y++;
                       }
              }
      $x++;
     }
}

function get_top_items($is_team, $appid, $is_total, $offset, $added_offset, $up) {
    global $apps;
    global $items_per_page;
    global $store;
    $fetched_items = $items_per_page;
    $check = 0;
    $fetch = 1;
    $used_offset = $offset + $added_offset;
    $x = $is_total?"total":"expavg";
    
    if ($is_team) {
       $data = choose_teams($x, $appid, $used_offset, $up);
       retrieve_credit_team($data);
    
    } else { 
      $data = choose_users($x, $appid, $used_offset, $up);
      retrieve_credit_user($data);
    }
 
  //What gets returned is an array containing all we need for the page in the correct order
  // 
  return $store;
}

$is_team = get_int('is_team', true);
$appid = get_int('appid', true);
$is_total = get_int('is_total', true);
$added_offset = get_int('added_offset', true);
$items_per_page = 20;
$offset = get_int('offset', true);

//1 for previous page and default for next
//
$up = get_int('up', true);

if (!$offset) {
    $offset=0;
    $added_offset = 0;
}

if ($offset % $items_per_page) {
    $offset = 0;
    $added_offset = 0;
}

$x = $is_team?"teams":"participants";
page_head(tra("Top %1 by application", $x));

$apps = BoincApp::enum("deprecated=0");
if (!$appid) {
    $appid = $apps[0]->id;
}

if ($offset < ITEM_LIMIT) {
     $cache_args = "appid=$appid&is_team=$is_team&is_total=$is_total&offset=$offset";
     $cacheddata = get_cached_data(TOP_PAGES_TTL,$cache_args);

     // Do we have the data in cache?
     //
     if ($cacheddata){
         $data = unserialize($cacheddata); // use the cached data
     } else {
        //if not do queries etc to generate new data
        $data = get_top_items($is_team, $appid, $is_total, $offset, $added_offset, $up);
 
        //save data in cache
        //
        set_cached_data(TOP_PAGES_TTL, serialize($data),$cache_args);
       }
} else {
    error_page(tra("Limit exceeded - Sorry, first %1 items only", ITEM_LIMIT));
  }


start_table('table_striped');
show_header($is_team, $apps, $appid, $is_total);

  $i = 1 + $offset;
  $n = $data[0][0];
  $added_offset = $data[1][0] +  $added_offset;
  
  for ($x = 1; $x <= $n; $x++) {
       show_row($data, $x);    
       $i++;
  }
  
end_table();

if ($offset > 0) {
    $new_offset = $offset - $items_per_page;
    echo "<a href=per_app_list.php?appid=$appid&is_team=$is_team&is_total=$is_total&added_offset=$added_offset&up=1&offset=$new_offset>".tra("Previous %1", $items_per_page)."</a> &middot; ";
}

if ($n == $items_per_page) {
    $new_offset = $offset + $items_per_page;
    echo "<a href=per_app_list.php?appid=$appid&is_team=$is_team&is_total=$is_total&added_offset=$added_offset&up=0&offset=$new_offset>".tra("Next %1", $items_per_page)."</a>";
}

page_tail();

?>
