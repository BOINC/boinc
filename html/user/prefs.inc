<?php

// This file contains support functions for display and editing
// preferences (global and project).
// Preferences are represented in two ways:
// - As a PHP structure (usually called $prefs)
// - As XML (usually called $prefs_xml)
// Various functions are defined below for converting between these forms,
// and also to/from HTML form elements

// functions to parse preferences XML into a struct
//
// This XML has the general structure
// <global_preferences>
//    <dont_run_if_user_active/>
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
        $prefs->project_specific = $text;
        $in_project_specific = false;
        break;
    case "dont_run_on_batteries":
        $parse_result->dont_run_on_batteries = 1;
        break;
    case "dont_run_if_user_active":
        $parse_result->dont_run_if_user_active = 1;
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
        $prefs->resource_share = $text;
        break;
    case "show_email":
        $prefs->show_email = $text;
        break;
    case "send_email":
        $prefs->send_email = $text;
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

function prefs_parse($prefs_xml) {
    global $parse_result;

    $parse_result = null;
    $parse_result->dont_run_on_batteries = 0;
    $parse_result->dont_run_if_user_active = 0;
    $parse_result->confirm_before_connecting = 0;
    $parse_result->low_water_days = 1;
    $parse_result->high_water_days = 3;
    $parse_result->disk_max_used_gb = 1000;
    $parse_result->disk_max_used_pct = 50;
    $parse_result->disk_min_free_gb = 1;

    $parse_result->resource_share = 1;
    $parse_result->show_email = 1;
    $parse_result->send_email = 1;

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

    echo "<p>";
    echo "<table width=580 cellpadding=4>\n";
    echo "<tr>".TD2.LG_FONT."<b>Global preferences:</b></font></td></tr>\n";
    if ($prefs->dont_run_on_batteries) {
        $batteries = "No";
    } else {
        $batteries = "Yes";
    }
    if ($prefs->dont_run_if_user_active) {
        $in_use = "No";
    } else {
        $in_use = "Yes";
    }
    if ($prefs->confirm_before_connecting) {
        $confirm = "Yes";
    } else {
        $confirm = "No";
    }
    row2a("<b>Work if computer on batteries:   </b>", $batteries);
    row2a("<b>Work if computer in use:   </b>", $in_use);
    row2a("<b>Confirm before connecting to network:   </b>", $confirm);
    row2a("<b>Minimum amount of work to buffer:   </b>", "$prefs->low_water_days days");
    row2a("<b>Maximum amount of work to buffer:   </b>", "$prefs->high_water_days days");
    
    
    echo "<p>";
    row2a("<b>Maximum disk space to use:   </b>", "$prefs->disk_max_used_gb GB");
    row2a("<b>Minimum disk space to leave free:   </b>", "$prefs->disk_min_free_gb GB");
    row2a("<b>Maximum % of disk allowed to used:   </b>", "$prefs->disk_max_used_pct %");
    echo "<tr><td><a href=prefs_edit_global_form.php>Edit global preferences</a></td></tr>\n";
    echo "</table>\n";
}

function show_project($prefs) {
    echo "<tr><td><hr></tr></td>";
    row2a("<b>Resource Share:   </b>", $prefs->resource_share);
    row2a("<b>Project Specific Preferences:   </b>", htmlspecialchars($prefs->project_specific));
}

function prefs_show_project($prefs) {
    echo "<p>";
    echo "<table width=580 cellpadding=4>\n";
    echo "<tr>".TD2.LG_FONT."<b>Project preferences:</b></font></td></tr>\n";
    show_project($prefs);
    echo "<tr><td><hr></td></tr>";
    echo "<tr><td><a href=prefs_edit_project_form.php>Edit project preferences</a></td></tr>\n";
    echo "</table>";
}

function print_prefs_display($prefs) {
    prefs_show_global($prefs);
    prefs_show_project($prefs);
}

////////////////////////////////////////////
//
// Functions to display preference subsets as forms
//
function prefs_form_global($user, $prefs) {
    echo   "<form action=prefs_edit_global_action.php>\n";
    echo   "    <table cellpadding=6>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Don't run if computer is on batteries</b></td>\n";
    printf("        <td><input type=checkbox name=dont_run_on_batteries %s></td>\n", $prefs->dont_run_on_batteries?"checked":"");
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Don't run if user is active</b></td>\n";
    printf("        <td><input type=checkbox name=dont_run_if_user_active %s></td>\n", $prefs->dont_run_if_user_active?"checked":"");
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Confirm before connecting</b></td>\n";
    printf("        <td><input type=checkbox name=confirm_before_connecting %s></td>\n", $prefs->confirm_before_connecting?"checked":"");
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Minimum amount of work to buffer (days)</b></td>\n";
    printf("        <td><input size=5 name=low_water_days value='$prefs->low_water_days'></td>\n");
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Maximum amount of work to buffer (days)</b></td>\n";
    printf("        <td><input size=5 name=high_water_days value='$prefs->high_water_days'></td>\n");
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Maximum disk space allowed<br>to be used for BOINC (in MB)</b></td>\n";
    echo   "        <td><input size=7 name=disk_max_used_gb value='$prefs->disk_max_used_gb'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Minimum disk space to leave<br>free (in MB)</b></td>\n";
    echo   "        <td><input size=7 name=disk_min_free_gb value='$prefs->disk_min_free_gb'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Maximum % of disk space to<br>use for BOINC</b></td>\n";
    echo   "        <td><input size=5 name=disk_max_used_pct value='$prefs->disk_max_used_pct'></td>\n";
    echo   "    </tr>\n";
    echo   "    </table>\n";
    echo   "    <input type=submit value=\"Edit Preferences\">\n";
    echo   "</form>\n";
}

function prefs_form_project($prefs) {
    echo   "<form action=prefs_edit_project_action.php>\n";
    echo   "    <table cellpadding=6>\n";
    echo   "    <tr>\n";
    
    echo   "        <td align=right><b>Resource share:</b><br><font size=-1> (If projects";
    echo   "        compete for resources,<br>this is the fraction of resources you<br>choose to";
    echo   "        allocate to this project.)</font></td>\n";
    
    echo   "        <td><input name=resource_share value='$prefs->resource_share'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Project-specific preferences:</b></td>\n";
    echo   "        <td><input name=project_specific value='$prefs->project_specific'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr><td><br></td><td><input type=submit value=OK></td></tr>\n";
    echo   "    </table>\n";
    echo   "    </form>\n";
}

////////////////////////////////////////////
//
// Functions to parse form elements, modifying a preferences structure
//
function prefs_global_parse_form(&$prefs) {
    parse_str(getenv("QUERY_STRING"));
    $prefs->dont_run_on_batteries = isset($dont_run_on_batteries)?1:0;
    $prefs->dont_run_if_user_active = isset($dont_run_if_user_active)?1:0;
    $prefs->confirm_before_connecting = isset($confirm_before_connecting)?1:0;
    $prefs->low_water_days = $low_water_days;
    $prefs->high_water_days = $high_water_days;
    $prefs->disk_max_used_gb = $disk_max_used_gb;
    $prefs->disk_max_used_pct = $disk_max_used_pct;
    $prefs->disk_min_free_gb = $disk_min_free_gb;
}

function prefs_project_parse_form(&$prefs) {
    parse_str(getenv("QUERY_STRING"));
    $prefs->resource_share = $resource_share;
    $prefs->project_specific = $project_specific;
}


////////////////////////////////////////////
//
// convert prefs from structure to XML
//
function global_prefs_make_xml($prefs) {
    $xml = "<global_preferences>\n";
    if ($prefs->dont_run_on_batteries) {
        $xml = $xml."<dont_run_on_batteries/>\n";
    }
    if ($prefs->dont_run_if_user_active) {
        $xml = $xml."<dont_run_if_user_active/>\n";
    }
    if ($prefs->confirm_before_connecting) {
        $xml = $xml."<confirm_before_connecting/>\n";
    }
    $xml = $xml."<low_water_days>$prefs->low_water_days</low_water_days>
        <high_water_days>$prefs->high_water_days</high_water_days>
        ";
    $xml = $xml."
        <disk_max_used_gb>$prefs->disk_max_used_gb</disk_max_used_gb>
        <disk_max_used_pct>$prefs->disk_max_used_pct</disk_max_used_pct>
        <disk_min_free_gb>$prefs->disk_min_free_gb</disk_min_free_gb>";
    $xml = $xml."</global_preferences>\n";
    return $xml;
}

function project_prefs_make_xml($prefs) {
    $xml = "<project_preferences>\n";
    $xml = $xml."
            <resource_share>$prefs->resource_share</resource_share>
            <project_specific>\n$prefs->project_specific\n</project_specific>";
    $xml = $xml."</project_preferences>\n";
}

////////////////////////////////////////////
//
// Update user's prefs in database, from a given structure
//
function global_prefs_update($user, $prefs) {
    $prefs_xml = global_prefs_make_xml($prefs);
    $now = time();
    mysql_query("update user set global_prefs='$prefs_xml', prefs_mod_time=$now where id=$user->id");
}

function project_prefs_update($user, $prefs) {
    $prefs_xml = project_prefs_make_xml($prefs);
    $now = time();
    mysql_query("update user set project_prefs='$prefs_xml' where id=$user->id");
}

?>
