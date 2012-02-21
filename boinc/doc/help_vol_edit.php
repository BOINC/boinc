<?php

require_once("docutil.php");
require_once("spoken_languages.php");
require_once("help_funcs.php");
require_once("help_db.php");
require_once("countries.inc");

function print_form($vol, $action_name) {
    list_start();
    list_item(
        "Name or nickname",
        input("volname", $vol->name)
    );
    list_item(
        "Email address<br><font size=-2>This won't be publicized,
        but user email may be sent here.
        Use a separate mailbox if you want.</font>",
        input("email_addr", $vol->email_addr)
    );
    list_item(
        "Password",
        password("password", $vol->password)
    );
    list_item(
        "Skype ID<br><font size=-2>
        This will be publicized.
        Use a Skype account other than
        your primary one if you want.</font>",
        input("skypeid", $vol->skypeid)
    );
    list_item(
        "Primary language",
        spoken_language_list("lang1", $vol->lang1)
    );
    list_item(
        "Secondary language",
        spoken_language_list("lang2", $vol->lang2)
    );
    list_item(
        "Country",
        "<select name=country>".country_select($vol->country)."</select>"
    );
    list_item(
        "Specialties<br><font size=-2>
        What kinds of computers (Windows/Mac/Linux)
        and/or networking technologies (proxies, NATs)
        are you most familiar with?</font>",
        textarea("specialties", $vol->specialties)
    );
    list_item(
        "Projects<br><font size=-2>
        Do you specialize in any particular BOINC-based projects?</font>",
        textarea("projects", $vol->projects)
    );
    list_item(
        "What days and times are you typically available for help?
        <br><font size=-2>Include your time zone, or use UTC</font>",
        textarea("availability", $vol->availability)
    );
    list_item(
        "Is Skype voice OK?",
        yesno("voice_ok", $vol->voice_ok)
    );
    list_item(
        "Is Skype text OK?",
        yesno("text_ok", $vol->text_ok)
    );
    list_item(
        "Hide your account?",
        yesno("hide", $vol->hide)
    );
    list_item(
        "",
        "<input type=submit name=$action_name value=OK>"
    );
    list_end();
}

function get_form_data() {
    $vol->name = stripslashes($_GET['volname']);
    if (!$vol->name) boinc_error_page("Name must not be blank");
    if (strstr($vol->name, "<")) boinc_error_page("No &lt; allowed");
    $vol->password = stripslashes($_GET['password']);
    if (!$vol->password) boinc_error_page("Password must not be blank");
    $vol->email_addr = stripslashes($_GET['email_addr']);
    if (!$vol->email_addr) boinc_error_page("Email address must not be blank");
    $vol->skypeid = stripslashes($_GET['skypeid']);
    if (!$vol->skypeid) boinc_error_page("Skype ID must not be blank");
    $vol->lang1 = stripslashes($_GET['lang1']);
    if (!$vol->lang1) boinc_error_page("Primary language must not be blank");
    if (!is_spoken_language($vol->lang1)) boinc_error_page("Not a language");
    $vol->lang2 = stripslashes($_GET['lang2']);
    if (!is_spoken_language($vol->lang2)) boinc_error_page("Not a language");
    $vol->country = stripslashes($_GET['country']);
    if (!is_valid_country($vol->country)) boinc_error_page("Bad country");
    $vol->specialties = stripslashes($_GET['specialties']);
    if (strstr($vol->specialties, "<")) boinc_error_page("No &lt; allowed");
    $vol->projects = stripslashes($_GET['projects']);
    if (strstr($vol->projects, "<")) boinc_error_page("No &lt; allowed");
    $vol->availability = stripslashes($_GET['availability']);
    if (strstr($vol->availability, "<")) boinc_error_page("No &lt; allowed");
    $vol->voice_ok = $_GET['voice_ok']?1:0;
    $vol->text_ok = $_GET['text_ok']?1:0;
    $vol->hide = $_GET['hide']?1:0;
    return $vol;
}

function email_password($vol) {
    page_head("Emailing password");
    echo "We're emailing your Help Volunteer password to $vol->email_addr.";
    page_tail();
    $body = "Your BOINC Help Volunteer password is:\n$vol->password\n";
    mail($vol->email_addr, "Help Volunteer info", $body, "From: BOINC");
}

$create = $_GET['create'];
$edit_login = $_GET['edit_login'];
$edit_form = $_GET['edit_form'];
$edit_action = $_GET['edit_action'];
if ($create == 'OK') {
    $vol = get_form_data();
    $vol->create_time = time();
    $vol->timezone = 99;
    
    $vol2 = vol_lookup_name($vol->name);
    if ($vol2) {
        boinc_error_page("That name is already taken");
    }
    $vol2 = vol_lookup_email($vol->email_addr);
    if ($vol2) {
        boinc_error_page("There's already an account with email address $vol->email_addr");
    }
    $retval = vol_insert($vol);
    if (!$retval) {
        echo mysql_error();
        page_head("database error");
    } else {
        page_head("Registration finished");
        echo "
            Thanks - you're now registered as a BOINC Help Volunteer.
            <p>
            If later you want to change your account information,
            go to the <a href=help.php>Help page</a>
            and click on the link at the bottom of the page.
        ";
    }
    page_tail();
} else if ($edit_login) {
    page_head("Edit your Help Volunteer account");
    echo "Please enter the email address and password
        of your Help Volunteer account.
        <form action=help_vol_edit.php>
    ";
    list_start();
    list_item("Email address", input("email_addr", ""));
    list_item("Password<br><font size=-2>If you forgot your password,
        leave this blank, and we'll email it to you.</font>",
        password("password", "")
    );
    list_item("", "<input type=submit name=edit_form value=OK>"
    );
    list_end();
    echo "</form>\n";
    page_tail();
} else if ($edit_form == "OK") {
    $email_addr = stripslashes($_GET['email_addr']);
    $password = stripslashes($_GET['password']);
    $vol = vol_lookup_email($email_addr);
    if (!$vol) {
        boinc_error_page("Bad email address $email_addr");
    }
    if (!$password) {
        email_password($vol);
        exit();
    }
    if ($password != $vol->password) {
        boinc_error_page("Bad password");
    }
    page_head("Edit your Help Volunteer Account");
    echo "
        <form action=help_vol_edit.php>
        <input type=hidden name=old_email_addr value=\"$email_addr\">
        <input type=hidden name=old_password value=\"$password\">
    ";
    print_form($vol, 'edit_action');
    echo "</form>\n";
    page_tail();
} else if ($edit_action) {
    $old_email_addr = stripslashes($_GET['old_email_addr']);
    $old_password = stripslashes($_GET['old_password']);
    $vol = vol_lookup_email($old_email_addr);
    if (!$vol) {
        boinc_error_page("Bad email address $old_email_addr");
    }
    if ($old_password != $vol->password) {
        boinc_error_page("Bad password");
    }
    $vol2 = get_form_data();
    $vol2->timezone = $vol->timezone;
    $vol2->id = $vol->id;
    $retval = vol_update($vol2);
    if (!$retval) {
        echo mysql_error();
        page_head("database error");
        page_tail();
    } else {
        page_head("Update completed");
        echo "Your Help Volunteer account information has been updated.";
        page_tail();
    }
} else {
    page_head("Register as a BOINC Help Volunteer");
    $vol = null;
    $vol->voice_ok = 1;
    $vol->text_ok = 1;
    echo "<form action=help_vol_edit.php>\n";
    print_form($vol, 'create');
    echo "</form>\n";
    page_tail();
}

?>
