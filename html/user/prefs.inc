<?php

// This file contains support functions for display and editing
// preferences (global and project).
// Preferences are represented in two ways:
// - As a PHP structure (usually called $prefs)
//   This has fields run_on_batteries, etc.
//   The fields "project_specific" is raw XML
// - As XML (usually called $prefs_xml)
//
// This XML has the general structure
// <global_preferences>
//    <run_if_user_active/>
//    <low_water_days>1.3</low_water_days>
//    ...
// </global_preferences>
//
// and
//
// <project_preferences>
//    <resource_share>4</resource_share>
//    <project-specific>
//            ... (arbitrary project-specific XML)
//    </project-specific>
//    <send_email/>
//    <show_email/>
// </project_preferences>

// Various functions are defined below for converting between these forms,
// and also to/from HTML form elements

// First: functions to parse preferences XML into a struct

include_once("project_specific_prefs.inc");

global $text;
global $parse_result;
global $in_project_specific;

// the following will parse either global or project prefs
// TODO: split up into separate functions
//
function element_start($parser, $name, $attrs) {
    global $text;
    global $project;
    global $in_project_specific;

    switch($name) {
    case "project_specific":
        $in_project_specific = 1;
        $text = "";
        break;
    default:
        if ($in_project_specific) {
            $text= $text."<$name>";
        } else {
            $text = "";
        }
    }
}

function element_end($parser, $name) {
    global $text;
    global $parse_result;
    global $in_project_specific;

    switch($name) {
    case "project_specific":
        $parse_result->project_specific = $text;
        $in_project_specific = false;
        break;
    case "run_on_batteries":
        $parse_result->run_on_batteries = true;
        break;
    case "run_if_user_active":
        $parse_result->run_if_user_active = true;
        break;
    case "confirm_before_connecting":
        $parse_result->confirm_before_connecting = 1;
        break;
    case "low_water_days":
        $parse_result->low_water_days = $text;
        break;
    case "high_water_days":
        $parse_result->high_water_days = $text;
        break;
    case "disk_max_used_gb":
        $parse_result->disk_max_used_gb = $text;
        break;
    case "disk_max_used_pct":
        $parse_result->disk_max_used_pct = $text;
        break;
    case "disk_min_free_gb":
        $parse_result->disk_min_free_gb = $text;
        break;
    case "resource_share":
        $parse_result->resource_share = $text;
        break;
    case "show_email":
        $parse_result->show_email = true;
        break;
    case "send_email":
        $parse_result->send_email = true;
        break;
    case "mod_time":
        $parse_result->mod_time = $text;
        break;
    case "global_preferences":
        break;
    case "project_preferences":
        break;
    default:
        if ($in_project_specific) {
            $text = $text."</$name>\n";
        } else {
            echo "Unknown tag: $name\n";
        }
    }
}

function char_handler($parser, $x) {
    global $text;
    $text = $text.$x;
}

// state of prefs for new users
//
function default_prefs() {
    $p = null;
    $p->run_on_batteries = false;
    $p->run_if_user_active = false;
    $p->confirm_before_connecting = 0;
    $p->low_water_days = 1;
    $p->high_water_days = 3;
    $p->disk_max_used_gb = 100;
    $p->disk_max_used_pct = 50;
    $p->disk_min_free_gb = 1;

    $p->resource_share = 100;
    $p->show_email = false;
    $p->send_email = true;
    return $p;
}

// state of prefs before parsing
//
function initial_prefs() {
    $p = default_prefs;
    $p->show_email = false;
    $p->send_email = false;
}

// parse prefs (either global or project) from XML to a struct
//
function prefs_parse($prefs_xml) {
    global $parse_result;
    
    $parse_result = initial_prefs();

    $xml_parser = xml_parser_create();
    xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, 0);
    xml_set_element_handler($xml_parser, "element_start", "element_end");
    xml_set_character_data_handler($xml_parser, "char_handler");
    xml_parse($xml_parser, $prefs_xml, 1);
    return $parse_result;
}

////////////////////////////////////////////
//
// display preference subsets, with Edit buttons
//
function prefs_show_global($prefs) {
    row2a("Work if computer on batteries:", $prefs->run_on_batteries?"Yes":"No");
    row2a("Work if computer in use:", $prefs->run_if_user_active?"Yes":"No");
    row2a("Confirm before connecting to network:", $prefs->confirm_before_connecting?"Yes":"No");
    row2a("Minimum amount of work to buffer:", "$prefs->low_water_days days");
    row2a("Maximum amount of work to buffer:", "$prefs->high_water_days days");
    row2a("Maximum disk space to use:", "$prefs->disk_max_used_gb GB");
    row2a("Minimum disk space to leave free:", "$prefs->disk_min_free_gb GB");
    row2a("Maximum % of disk allowed to used:", "$prefs->disk_max_used_pct %");
}

function prefs_show_resource($prefs) {
    row2(
        "Resource share:
        <br><font size=-1>If you participate in multiple BOINC projects, this is the proportion of your resources used by ".PROJECT."</font>",
        $prefs->resource_share
    );
}

function prefs_show_email($prefs) {
    row2("Should ".PROJECT." send you email newsletters?", $prefs->send_email?"Yes":"No");
    row2("Should ".PROJECT." show your email address on its web site?", $prefs->show_email?"Yes":"No");
}

function prefs_show_project($prefs) {
    project_specific_prefs_show($prefs);
}

function print_prefs_display($user) {
    echo "<table width=580 cellpadding=4>\n";
    $global_prefs = prefs_parse($user->global_prefs);
    $project_prefs = prefs_parse($user->project_prefs);
    $project_specific_prefs = project_specific_prefs_parse($project_prefs->project_specific);

    echo "<tr><td colspan=2><b>".PROJECT." preferences</b></td></tr>\n";
    prefs_show_resource($project_prefs);
    prefs_show_project($project_specific_prefs);
    prefs_show_email($project_prefs);
    venue_show($user);
    echo "<tr><td><a href=prefs_edit_project_form.php>Edit ".PROJECT." preferences</a></td></tr>\n";
    echo "<tr><td colspan=2><hr></td></tr>\n";
    echo "<tr><td colspan=2><b>BOINC preferences</b></font>
        <br><font size=-1>These apply to all BOINC projects in which you participate</font></td></tr>\n";
    prefs_show_global($global_prefs);
    echo "<tr><td><a href=prefs_edit_global_form.php>Edit BOINC preferences</a></td></tr>\n";
    echo "</table>";
}

////////////////////////////////////////////
//
// Functions to display preference subsets as forms
//
function prefs_form_global($user, $prefs) {
    echo   "<tr>
        <td align=right>Should ".PROJECT." run while the computer is on battery power?
        <br><font size=-1>(This matters only for portable computers)</font>
        </td><td valign=top>
    ";
    printf("Yes <input type=radio name=run_on_batteries value=yes %s>\n", $prefs->run_on_batteries?"checked":"");
    printf("No <input type=radio name=run_on_batteries value=no %s>\n", $prefs->run_on_batteries?"":"checked");
    echo   "</td></tr>
        <tr>
        <td align=right>Should ".PROJECT." run while you're using the computer?
        </td><td valign=top>
    ";
    printf("Yes <input type=radio name=run_if_user_active value=yes %s>\n", $prefs->run_if_user_active?"checked":"");
    printf("No <input type=radio name=run_if_user_active value=no %s>\n", $prefs->run_if_user_active?"":"checked");
    echo   "</td></tr>
        <tr>
        <td align=right>Ask you before connecting to Internet?
        <br><font size=-1>(This matters only if you use a modem)</font>
        </td><td valign=top>
    ";
    printf("Yes <input type=radio name=confirm_before_connecting %s>\n", $prefs->confirm_before_connecting?"checked":"");
    printf("No <input type=radio name=confirm_before_connecting %s>\n", $prefs->confirm_before_connecting?"":"checked");
    echo   "</td></tr>
        <tr>
        <td align=right>Keep enough to work on disk to last between
        </td><td>
        <input size=5 name=low_water_days value='$prefs->low_water_days'>
        and
        <input size=5 name=high_water_days value='$prefs->high_water_days'> days
        </td></tr>
        <tr><td colspan=2>
        You can limit the disk space used by ".PROJECT." in three different ways:
        </td></tr>
        <tr>
        <td align=right valign=top>Use no more than</td>
        <td><input size=7 name=disk_max_used_gb value='$prefs->disk_max_used_gb'> Gbytes</td>
        </tr>
        <tr>
        <td align=right valign=top>Leave at least</td>
        <td><input size=7 name=disk_min_free_gb value='$prefs->disk_min_free_gb'> Gbytes free</td>
        </tr>
        <tr>
        <td align=right valign=top>Use no more than</td>
        <td><input size=5 name=disk_max_used_pct value='$prefs->disk_max_used_pct'> % of total space</td>
        </tr>
    ";
}

function prefs_form_email($prefs) {
    echo "<tr>
        <td align=right>Should ".PROJECT." send you email newsletters?</td><td>
    ";
    printf("Yes <input type=radio name=send_email value=yes %s>\n", $prefs->send_email?"checked":"");
    printf("No <input type=radio name=send_email value=no %s>\n", $prefs->send_email?"":"checked");
     echo "   </td></tr><tr>
        <td align=right>Should ".PROJECT." show your email address on its web site?</td><td>
    ";
    printf("Yes <input type=radio name=show_email value=yes %s>\n", $prefs->show_email?"checked":"");
    printf("No <input type=radio name=show_email value=no %s>\n", $prefs->show_email?"":"checked");
    echo" </td></tr>
    ";
}

function prefs_form_resource($prefs) {
    echo "<tr>
        <td width=50% valign=top align=right><b>Resource share:</b>
        <font size=-1><br>The proportion of your computer's resources
        (processing time and disk space)
        allocated to ".PROJECT."
        relative to the other BOINC projects in which you participate.
        The default is 100.
        For example, if you participate in two projects and
        give them resource shares of 100 and 200,
        the first will get 1/3 of our resources and the second will get 2/3.
        </font></td>
        <td valign=top><input name=resource_share value='$prefs->resource_share'></td>
    ";
}

function prefs_form_project($prefs_xml) {
    $prefs = project_specific_prefs_parse($prefs_xml);
    project_specific_prefs_edit($prefs);
}

function venue_show($user) {
    echo "<tr><td>Venue</td><td>$user->venue</td></tr>\n";
}

function venue_form($user) {
    if ($user->venue == "home") $h = "selected";
    if ($user->venue == "work") $w = "selected";
    if ($user->venue == "school") $s = "selected";
    echo "<tr><td align=right>Computer location</td><td>
        <select name=venue>
        <option value=home $h>Home
        <option value=work $w>Work
        <option value=school $s>School
        </select>
        </td></tr>
        ";
}

function venue_parse(&$user) {
    parse_str(getenv("QUERY_STRING"));
    $user->venue = $venue;
}

function venue_update($user) {
    mysql_query("update user set venue='$user->venue' where id=$user->id");
}

////////////////////////////////////////////
//
// Functions to parse form elements, modifying a preferences structure
//
function prefs_global_parse_form(&$prefs) {
    parse_str(getenv("QUERY_STRING"));
    $prefs->run_on_batteries = ($run_on_batteries == "yes");
    $prefs->run_if_user_active = ($run_if_user_active == "yes");
    $prefs->confirm_before_connecting = isset($confirm_before_connecting)?1:0;
    $prefs->low_water_days = $low_water_days;
    $prefs->high_water_days = $high_water_days;
    $prefs->disk_max_used_gb = $disk_max_used_gb;
    $prefs->disk_max_used_pct = $disk_max_used_pct;
    $prefs->disk_min_free_gb = $disk_min_free_gb;
}

function prefs_resource_parse_form(&$prefs) {
    parse_str(getenv("QUERY_STRING"));
    $prefs->resource_share = $resource_share;
}

function prefs_email_parse_form(&$prefs) {
    parse_str(getenv("QUERY_STRING"));
    $prefs->send_email = ($send_email == "yes");
    $prefs->show_email = ($show_email == "yes");
}

function prefs_project_parse_form(&$prefs) {
    $prefs->project_specific = project_specific_prefs_parse_form();
}


////////////////////////////////////////////
//
// convert prefs from structure to XML
//
function global_prefs_make_xml($prefs) {
    $xml = "<global_preferences>\n";
    $now = time();
    $xml = $xml."<mod_time>$now</mod_time>\n";
    if ($prefs->run_on_batteries) {
        $xml = $xml."<run_on_batteries/>\n";
    }
    if ($prefs->run_if_user_active) {
        $xml = $xml."<run_if_user_active/>\n";
    }
    if ($prefs->confirm_before_connecting) {
        $xml = $xml."<confirm_before_connecting/>\n";
    }
    $xml = $xml
        ."<low_water_days>$prefs->low_water_days</low_water_days>\n"
        ."<high_water_days>$prefs->high_water_days</high_water_days>\n";
    $xml = $xml
        ."<disk_max_used_gb>$prefs->disk_max_used_gb</disk_max_used_gb>\n"
        ."<disk_max_used_pct>$prefs->disk_max_used_pct</disk_max_used_pct>\n"
        ."<disk_min_free_gb>$prefs->disk_min_free_gb</disk_min_free_gb>\n";
    $xml = $xml."</global_preferences>\n";
    return $xml;
}

function project_prefs_make_xml($prefs) {
    $xml = "<project_preferences>\n";
    if ($prefs->show_email == 1) {
        $xml = $xml."<show_email/>\n";
    }
    if ($prefs->send_email == 1) {
        $xml = $xml."<send_email/>\n";
    }
    $xml = $xml
        ."<resource_share>$prefs->resource_share</resource_share>\n"
        ."<project_specific>\n$prefs->project_specific</project_specific>\n";
    $xml = $xml."</project_preferences>\n";
    return $xml;
}

////////////////////////////////////////////
//
// Update user's prefs in database, from a given structure
//
function global_prefs_update(&$user, $prefs) {
    $prefs_xml = global_prefs_make_xml($prefs);
    mysql_query("update user set global_prefs='$prefs_xml' where id=$user->id");
    $user->global_prefs = $prefs_xml;
}

function project_prefs_update(&$user, $prefs) {
    $prefs_xml = project_prefs_make_xml($prefs);
    mysql_query("update user set project_prefs='$prefs_xml' where id=$user->id");
    $user->project_prefs = $prefs_xml;
}

?>
