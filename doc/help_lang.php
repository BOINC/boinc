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

function order_vols($vols) {
    $online = array();
    $offline = array();
    foreach ($vols as $vol) {
        if (online($vol->status)) {
            $online[] = $vol;
        } else {
            $offline[] = $vol;
        }
    }
    shuffle($online);
    return array_merge($online, $offline);
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

function rating_info($vol) {
    if ($vol->nratings == 0) {
        return "<font size=-2>(no ratings)</font>";
    }
    $x = $vol->rating_sum/$vol->nratings;
    if ($x > 4.5) $img = "stars-5-0.gif";
    else if ($x > 4.0) $img = "stars-4-5.gif";
    else if ($x > 3.5) $img = "stars-4-0.gif";
    else if ($x > 3.0) $img = "stars-3-5.gif";
    else if ($x > 2.5) $img = "stars-3-0.gif";
    else if ($x > 2.0) $img = "stars-2-5.gif";
    else if ($x > 1.5) $img = "stars-2-0.gif";
    else if ($x > 1.0) $img = "stars-1-5.gif";
    else if ($x > 0.5) $img = "stars-1-0.gif";
    else if ($x > 0.0) $img = "stars-0-5.gif";
    else $img = "stars-0-0.gif";
    return "
        <nobr><a href=help_ratings.php?volid=$vol->id>
        <img border=0 src=images/help/$img>
        <font size=-2>
        $vol->nratings ratings</font></a></nobr>
    ";
}

function info($vol) {
    global $lang;
    $x = "<font size=-1> Country: $vol->country\n";
    if ($vol->availability) {
        $x .= "<br>Usual hours: $vol->availability";
    }
    if ($vol->specialties) {
        $x .= "<br>Specialties: $vol->specialties";
    }
    if ($vol->projects) {
        $x .= "<br>Projects: $vol->projects";
    }
    if (!$lang) {
        $x .= "<br> Primary language: $vol->lang1";
        if (!$vol->lang2) {
            $x .= "<br> Secondary language: $vol->lang2";
        }
    }
    $x .= "</font>";
    return $x;
}

function show_vol($vol) {
    $status = $vol->status;
    $image = button_image($status);
    list_item_array(array(
        vol_info($vol),
        status_string($status),
        vol_modes($vol),
        info($vol),
        rating_info($vol)
    ));
}

function show_vols($vols) {
    echo "<p><font size=-1>
        You can send email to a volunteer even if they're offline.
        To do so, click their name.
        <p>
        Volunteers can answer questions about the BOINC client software,
        not about the server software.
        If you're setting up a BOINC project,
        this is not the place to get help.
        Instead, try the
        <a href=http://boinc.berkeley.edu/email_lists.php>boinc_projects</a>
        email list.
        </font>
    ";
    list_start("border=0");
    list_heading_array(array(
        "Volunteer name<br><font size=2>click to contact</font>",
        "Status",
        "Voice/Text",
        "Info",
        "Feedback <br><font size=-2>Click to see comments</font>",
    ));
    foreach ($vols as $vol) {
        show_vol($vol);
    }
    list_end();
}

if ($lang) {
    page_head("Online Help in $lang");
} else {
    page_head("Online Help in all languages");
}
$vols = get_vols($lang);
$vols = order_vols($vols);
show_vols($vols);
page_tail();
?>
