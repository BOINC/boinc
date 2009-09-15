<?php

ini_set("memory_limit", "1023M");

require_once("../inc/util_ops.inc");

BoincDb::get();

$profiles = BoincProfile::enum("");

foreach ($profiles as $p) {
    $id = $p->userid;
    $path = "../user_profile/images/$id.jpg";
    $smpath = "../user_profile/images/".$id."_sm.jpg";
    $has_pic = file_exists($path);
    $has_pic_sm = file_exists($smpath);
    if ($p->has_picture) {
        if (!$has_pic || !$has_pic_sm) {
            echo "$id $p->has_picture $has_pic $has_pic_sm\n";
            BoincProfile::update_aux("has_picture=0 where userid=$id");
        }
    } else {
        if ($has_pic && $has_pic_sm) {
            echo "$id $p->has_picture $has_pic $has_pic_sm\n";
            BoincProfile::update_aux("has_picture=1 where userid=$id");
        }
    }
}

?>
