<?php

// This file contains support functions for preference display and editing
// Preferences are represented in two ways:
// - As a PHP structure (usually called $prefs)
// - As XML (usually called $prefs_xml)
// Various functions are defined below for converting between these forms,
// and also to/from HTML form elements

// functions to parse preferences XML into a struct
//
// This XML has the general structure
// <preferences>
//    <dont_run_if_user_active/>
//    <low_water_days>1.3</low_water_days>
//    ...
//    <project>
//        <url>...</url>
//        <resource_share>45</resource_share>
//        ...
//        <project-specific>
//            ... (arbitrary project-specific XML)
//        </project-specific>
//    </project>
//    ... (other projects)
// </preferences>

global $text;
global $parse_result;
global $project;
global $in_project_specific;

function element_start($parser, $name, $attrs) {
    global $text;
    global $project;
    global $in_project_specific;

    switch($name) {
    case "project":
        $project = null;
        $text = "";
        break;
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
    global $project;
    global $parse_result;
    global $in_project_specific;

    switch($name) {
    case "project":
        $n = sizeof($parse_result->projects);
        $parse_result->projects[$n] = $project;
        break;
    case "project_specific":
        $project->project_specific = $text;
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
    case "master_url":
        $project->master_url = $text;
        break;
    case "email_addr":
        $project->email_addr = $text;
        break;
    case "authenticator":
        $project->authenticator = $text;
        break;
    case "resource_share":
        $project->resource_share = $text;
        break;
    case "show_email":
        $project->show_email = $text;
        break;
    case "send_email":
        $project->send_email = $text;
        break;
    case "preferences":
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
    $parse_result->disk_max_used_gb = 1000;
    $parse_result->disk_max_used_pct = 50;
    $parse_result->disk_min_free_gb = 1;

    $xml_parser = xml_parser_create();
    xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, 0);
    xml_set_element_handler($xml_parser, "element_start", "element_end");
    xml_set_character_data_handler($xml_parser, "char_handler");
    xml_parse($xml_parser, $prefs_xml, 1);
    return $parse_result;
}

////////////////////////////////////////////
//
// Functions to display preference subsets, with Edit buttons
//
function prefs_show_work($prefs) {
    echo "<h3>Work preferences</h3>\n";
    if ($prefs->dont_run_on_batteries) {
        echo "<br>Don't do work if running on batteries";
    } else {
        echo "<br>Do work even if running on batteries";
    }
    if ($prefs->dont_run_if_user_active) {
        echo "<br>Don't do work if computers is in use";
    } else {
        echo "<br>Do work even if computer is in user";
    }
    if ($prefs->confirm_before_connecting) {
        echo "<br>Confirm before connecting to network";
    } else {
        echo "<br>Don't confirm before connecting to network";
    }
    echo "<br>Buffer at least $prefs->low_water_days hours of work\n";
    echo "<br>Buffer at most $prefs->high_water_days hours of work\n";
    echo "<br><a href=prefs_edit_work_form.php>Edit work preferences</a>\n";
}

function prefs_show_disk($prefs) {
    echo "<h3>Disk preferences</h3>\n";
    echo "<br>Maximum disk used: $prefs->disk_max_used_gb GB\n";
    echo "<br>Minimum disk free: $prefs->disk_min_free_gb GB\n";
    echo "<br>Maximum % of disk used: $prefs->disk_max_used_pct\n";
    echo "<br><a href=prefs_edit_disk_form.php>Edit disk preferences</a>\n";
}

function prefs_show_project($project) {
    echo "<br>master_url: $project->master_url";
    echo "<br>email_addr: $project->email_addr";
    echo "<br>authenticator: $project->authenticator";
    echo "<br>resource_share: $project->resource_share";
    echo "<br>project_specific: ".htmlspecialchars($project->project_specific);
}

function prefs_show_projects($prefs) {
    echo "<h3>Project preferences</h3>\n";
    for ($i=0; $i<sizeof($prefs->projects); $i++) {
        $project = $prefs->projects[$i];
        prefs_show_project($project);
        echo "<br><br>\n";
    }
    echo "<br><a href=prefs_edit_projects.php>Edit project preferences</a>\n";
}

function print_prefs_display($prefs) {
    echo "<h2>Preferences</h2>\n";
    prefs_show_work($prefs);
    prefs_show_disk($prefs);
    prefs_show_projects($prefs);
}

////////////////////////////////////////////
//
// Functions to display preference subsets as forms
//
function prefs_form_work($user, $prefs) {
    echo   "<form action=prefs_edit_work_action.php>\n";
    echo   "    <table border=1 cellpadding=6>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>don't run if on batteries</td>\n";
    printf("        <td><input type=checkbox name=dont_run_on_batteries %s></td>\n", $prefs->dont_run_on_batteries?"checked":"");
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>don't run if user is active</td>\n";
    printf("        <td><input type=checkbox name=dont_run_if_user_active %s></td>\n", $prefs->dont_run_if_user_active?"checked":"");
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>confirm before connecting</td>\n";
    printf("        <td><input type=checkbox name=confirm_before_connecting %s></td>\n", $prefs->confirm_before_connecting?"checked":"");
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>Minimum work buffer (days)</td>\n";
    printf("        <td><input name=low_water_days value='$prefs->low_water_days'></td>\n");
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>Maximum work buffer (days)</td>\n";
    printf("        <td><input name=high_water_days value='$prefs->high_water_days'></td>\n");
    echo   "    </tr>\n";
    echo   "    <tr><td><br></td><td><input type=submit value=OK></td></tr>\n";
    echo   "    </table>\n";
    echo   "</form>\n";
}

function prefs_form_disk($user, $prefs) {
    echo   "<form action=prefs_edit_disk_action.php>\n";
    echo   "    <table border=1 cellpadding=6>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>Maximum disk space to use for BOINC</td>\n";
    echo   "        <td><input name=disk_max_used_gb value='$prefs->disk_max_used_gb'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>Minimum disk space to leave free</td>\n";
    echo   "        <td><input name=disk_min_free_gb value='$prefs->disk_min_free_gb'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>Maximum % of disk space to use for BOINC</td>\n";
    echo   "        <td><input name=disk_max_used_pct value='$prefs->disk_max_used_pct'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr><td><br></td><td><input type=submit value=OK></td></tr>\n";
    echo   "    </table>\n";
    echo   "</form>\n";
}

function prefs_form_projects($prefs) {
    for ($i=0; $i<sizeof($prefs->projects); $i++) {
        $project = $prefs->projects[$i];
        echo "<hr>\n";
        prefs_show_project($project);
        echo "<br><a href=prefs_delete_project_confirm.php?master_url=$project->master_url>Delete this project</a>\n";
        echo "<br><a href=prefs_edit_project_form.php?master_url=$project->master_url>Edit this project</a>\n";
    }
    echo "<hr><a href=prefs_add_project_form.php>Add a project</a>\n";
    echo "<br>";
}

function prefs_form_project($project, $action) {
    echo   "<form action=$action>\n";
    echo   "    <table border=1 cellpadding=6>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>Master URL</td>\n";
    echo   "        <td><input name=master_url value='$project->master_url'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>email address</td>\n";
    echo   "        <td><input name=email_addr value='$project->email_addr'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>Authenticator</td>\n";
    echo   "        <td><input name=authenticator value='$project->authenticator'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>Resource share</td>\n";
    echo   "        <td><input name=resource_share value='$project->resource_share'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right>Project-specific preferences</td>\n";
    echo   "        <td><input name=project_specific value='$project->project_specific'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr><td><br></td><td><input type=submit value=OK></td></tr>\n";
    echo   "    </table>\n";
}

////////////////////////////////////////////
//
// Functions to parse form elements, modifying a preferences structure
//
function prefs_work_parse_form(&$prefs) {
    parse_str(getenv("QUERY_STRING"));
    $prefs->dont_run_on_batteries = isset($dont_run_on_batteries)?1:0;
    $prefs->dont_run_if_user_active = isset($dont_run_if_user_active)?1:0;
    $prefs->confirm_before_connecting = isset($confirm_before_connecting)?1:0;
    $prefs->low_water_days = $low_water_days;
    $prefs->high_water_days = $high_water_days;
}

function prefs_disk_parse_form(&$prefs) {
    parse_str(getenv("QUERY_STRING"));
    $prefs->disk_max_used_gb = $disk_max_used_gb;
    $prefs->disk_max_used_pct = $disk_max_used_pct;
    $prefs->disk_min_free_gb = $disk_min_free_gb;
}

function prefs_project_parse_form(&$project) {
    parse_str(getenv("QUERY_STRING"));
    $project->master_url = $master_url;
    $project->email_addr = $email_addr;
    $project->authenticator = $authenticator;
    $project->resource_share = $resource_share;
    $project->project_specific = $project_specific;
}


////////////////////////////////////////////
//
// convert prefs from structure to XML
//
function prefs_make_xml($prefs) {
    $xml = "<preferences>\n";
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
    for ($i=0; $i<sizeof($prefs->projects); $i++) {
        $project = $prefs->projects[$i];
        $xml = $xml."
            <project>
            <master_url>$project->master_url</master_url>
            <email_addr>$project->email_addr</email_addr>
            <authenticator>$project->authenticator</authenticator>
            <resource_share>$project->resource_share</resource_share>
            <project_specific>\n$project->project_specific\n</project_specific>
            </project>";
    }
    $xml = $xml."</preferences>\n";
    return $xml;
}

////////////////////////////////////////////
//
// Update user's prefs in database, from a given structure
//
function prefs_update($user, $prefs) {
    $prefs_xml = prefs_make_xml($prefs);
    $now = time();
    mysql_query("update user set prefs='$prefs_xml', prefs_mod_time=$now where id=$user->id");
}


////////////////////////////////////////////
//
// Misc
//
function project_index($prefs, $master_url) {
    for ($i=0; $i<sizeof($prefs->projects); $i++) {
        $project = $prefs->projects[$i];
        if ($project->master_url == $master_url) {
            return $i;
        }
    }
    return -1;
}

?>
