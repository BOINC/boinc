<?php

// repair user.global_prefs fields that were corrupted
// (non-parsable XML) by a bug at some point

require_once("../inc/boinc_db.inc");

// insert "\n</venue>\n" before "</global_preferences>"
// This fixes an XML error introduced at some point in the past
//
function repair_prefs($prefs) {
    $x = strstr($prefs, "</global_preferences>", true);
    if (!$x) return null;
    return "$x\n    </venue>\n</global_preferences>\n";
}

function process_set($users) {
    foreach ($users as $user) {
        if (!$user->global_prefs) {
            //echo "$user->id: no prefs\n";
            continue;
        }
        $retval = @simplexml_load_string($user->global_prefs);
        if ($retval) {
            //echo "$user->id: good\n";
        } else {
            echo "$user->id: prefs don't parse\n";
            $p = repair_prefs($user->global_prefs);
            if ($p) {
                $retval = @simplexml_load_string($p);
                if ($retval) {
                    $user->update("global_prefs='$p'");
                    echo "repair succeeded for $user->id\n";
                } else {
                    echo "couldn't repair prefs for user $user->id\n";
                }
            } else {
                echo "bad prefs for user $user->id\n";
            }
        }
    }
}


$n = 0;
while (1) {
    $users = BoincUser::enum("true limit $n,1000");
    echo "processing from $n\n";
    if (!$users) break;
    process_set($users);
    $n += sizeof($users);
}
?>
