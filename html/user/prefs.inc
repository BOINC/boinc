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

    echo "<p>";
    echo TABLE2."\n";
    echo "<tr>".TD2.LG_FONT."<b>Work preferences:</b></font></td></tr>\n";
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
    row("<b>Work if computer on batteries:   </b>", $batteries);
    row("<b>Work if computer in use:   </b>", $in_use);
    row("<b>Confirm before connecting to network:   </b>", $confirm);
    row("<b>Minimum amount of work to buffer:   </b>", "$prefs->low_water_days hours");
    row("<b>Maximum amount of work to buffer:   </b>", "$prefs->high_water_days hours");
    echo "<tr><td><a href=prefs_edit_work_form.php>Edit work preferences</a></td></tr>\n";
    echo "</table>\n";
}

function prefs_show_disk($prefs) {

    echo "<p>";
    echo TABLE2."\n";
    echo "<tr>".TD2.LG_FONT."<b>Disk preferences:</b></font></td></tr>\n";
    row("<b>Maximum disk space allowed to be used:   </b>", "$prefs->disk_max_used_gb MB");
    row("<b>Minimum disk space to leave free:   </b>", "$prefs->disk_min_free_gb MB");
    row("<b>Maximum % of disk allowed to used:   </b>", "$prefs->disk_max_used_pct %");
    echo "<tr><td><a href=prefs_edit_disk_form.php>Edit disk preferences</a></td></tr>\n";
    echo "</table>\n";
}

function prefs_show_project($project) {
    echo "<tr><td><hr></tr></td>";
    row("<b>Master URL:   </b>", "$project->master_url");
    row("<b>Email Address:   </b>", $project->email_addr);
    row("<b>Authenticator:   </b>", $project->authenticator);
    row("<b>Resource Share:   </b>", $project->resource_share);
    row("<b>Project Specific Preferences:   </b>", htmlspecialchars($project->project_specific));
}

function prefs_show_projects($prefs) {

    echo "<p>";
    echo TABLE2."\n";
    echo "<tr>".TD2.LG_FONT."<b>Project preferences:</b></font></td></tr>\n";
    for ($i=0; $i<sizeof($prefs->projects); $i++) {
        $project = $prefs->projects[$i];
        prefs_show_project($project);
    }
    echo "<tr><td><hr></td></tr>";
    echo "<tr><td><a href=prefs_edit_projects.php>Edit project preferences</a></td></tr>\n";
    echo "</table>";
}

function print_prefs_display($prefs) {
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
    echo   "    <table cellpadding=6>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Don't run if computer is<br>on batteries</b></td>\n";
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
    echo   "        <td align=right><b>Minimum amount of work<br>to buffer (in hours)</b></td>\n";
    printf("        <td><input size=5 name=low_water_days value='$prefs->low_water_days'></td>\n");
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Maximum amount of work<br>to buffer (in hours)</b></td>\n";
    printf("        <td><input size=5 name=high_water_days value='$prefs->high_water_days'></td>\n");
    echo   "    </tr>\n";
    echo   "    </table>\n";
    echo   "    <input type=submit value=\"Edit Preferences\">\n";
    echo   "</form>\n";
}

function prefs_form_disk($user, $prefs) {
    echo   "<form action=prefs_edit_disk_action.php>\n";
    echo   "    <table cellpadding=6>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Maximum disk space allowed<br>to be used for BOINC(in MB)</b></td>\n";
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

function prefs_form_projects($prefs) {
    for ($i=0; $i<sizeof($prefs->projects); $i++) {
        $project = $prefs->projects[$i];
        echo "<table>";
        prefs_show_project($project);
        echo "<tr><td><a href=prefs_edit_project_form.php?master_url=$project->master_url>Edit project</a></td></tr>\n";
        echo "<tr><td><a href=prefs_delete_project_confirm.php?master_url=$project->master_url>Delete project</a></td></tr>\n";
        echo "</table>";
    }
    echo "<hr><a href=prefs_add_project_form.php>Add project</a>\n";
    echo "<p>";
    echo "<table width=780><tr><td>";
    echo "Whenever you join a new project, remember to add that project with the authenticator that you get from that project to your home project (the first project) you joined.";
    echo "</td></tr></table>";
}

function prefs_form_project($project, $action) {
    echo   "<form action=$action>\n";
    echo   "    <table cellpadding=6>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Master URL</b></td>\n";
    echo   "        <td><input size=60 name=master_url value='$project->master_url'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Email address</b></td>\n";
    echo   "        <td><input size=40 name=email_addr value='$project->email_addr'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Authenticator</b></td>\n";
    echo   "        <td><input size=32 name=authenticator value='$project->authenticator'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Resource share</b></td>\n";
    echo   "        <td><input name=resource_share value='$project->resource_share'></td>\n";
    echo   "    </tr>\n";
    echo   "    <tr>\n";
    echo   "        <td align=right><b>Project-specific preferences</b></td>\n";
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
