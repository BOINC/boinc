<?php

require_once("docutil.php");
require_once("help_db.php");
require_once("help_funcs.php");

$lang = $_GET["lang"];

function vol_info($vol) {
    $id = $vol->id;
    $x = "<a href=help_vol.php?volid=$id>$vol->name</a>";
    return $x;
}

function vol_modes($vol) {
    $x = "";
    if ($vol->voice_ok && $vol->text_ok) {
        return "Either";
    }
    if ($vol->text_ok) {
        return "Text only";
    }
    if ($vol->voice_ok) {
        return "Voice only";
    }
}

function show_vol($vol) {
    $status = skype_status($vol->skypeid);
    $image = button_image($status);
    list_item_array(array(
        vol_info($vol),
        vol_modes($vol),
        status_string($status),
        $vol->specialties,
        $vol->availability,
        "<img border=0 src=images/help/stars-5-0.gif>",
    ));
}

function show_vols($vols) {
    list_start("border=0");
    list_heading_array(array(
        "Volunteer name<br><font size=2>click to contact</font>",
        "Voice/Text",
        "Status",
        "Specialties",
        "Usual hours",
        "Average rating",
    ));
    foreach ($vols as $vol) {
        show_vol($vol);
    }
    list_end();
}

page_head("Online Help in $lang");
$vols = get_vols($lang);
show_vols($vols);
page_tail();
?>
