<?php

// repair user.global_prefs fields that were corrupted
// (non-parsable XML) by a bug at some point.
//
// This fixes 2 types of corruption:
// - missing </venue> tag before closing </global_preferences>
// - \" instead of "

require_once("../inc/boinc_db.inc");

// insert "\n</venue>\n" before "</global_preferences>"
// This fixes an XML error introduced at some point in the past
//
function repair_prefs($prefs) {
    if (strstr($prefs, '\"')) {
        return str_replace('\"', '"', $prefs);
    }
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
            echo "repairing prefs for user $user->id\n";
            $p = repair_prefs($user->global_prefs);
            if ($p) {
                $xml_obj = @simplexml_load_string($p);
                if ($xml_obj) {
                    // increase mod_time by 1 second so new preferences are propagated to the Client
                    $xml_obj->mod_time = 1 + intval($xml_obj->mod_time);
                    $p = $xml_obj->asXML();
                    // remove XML header
                    $p = implode("\n", array_slice(explode("\n", $p), 1));
                    $user->update("global_prefs='$p'");
                    echo "   repair succeeded\n";
                } else {
                    echo "   repair failed\n";
                }
            } else {
                echo "   prefs are missing end tag\n";
            }
        }
    }
}


$n = 0;
$maxid = BoincUser::max("id");
while ($n <= $maxid) {
    $m = $n + 1000;
    $users = BoincUser::enum("id >= $n and id < $m");
    //echo "processing from $n\n";
    if (!$users) break;
    process_set($users);
    $n = $m;
}
?>
