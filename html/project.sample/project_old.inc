<?php

// This is a sample project file.
// Modify it to suit your project.
// Put your version in html/project/project.inc
// add tra() around visible strings to make them translatable
// see inc/translation.inc and https://boinc.berkeley.edu/trac/wiki/TranslateProject for Details

require_once("../inc/util.inc");

$master_url = parse_config(get_config(), "<master_url>");
define("PROJECT", "Project-Rain");
//Disable forums?
//define("DISABLE_FORUMS", true);
//Prevent spam account creation
define("USE_STOPFORUMSPAM", true);
//No fun!
define("DISABLE_BADGES", true);
//Implement SSL
//define("SECURE_URL_BASE", "https://project-rain.co.uk/");
define("URL_BASE", $master_url);
define("IMAGE_PATH", "../user_profile/images/");
define("IMAGE_URL", "user_profile/images/");
define("PROFILE_PATH", "../user_profile/");
define("PROFILE_URL", "user_profile/");
define("LANGUAGE_FILE", "languages.txt");
define("STYLESHEET", "white.css");
//define("STYLESHEET2", "xxx.css");
define("FORUM_QA_MERGED_MODE", false); // Set to true to merge Message boards and Q&A section
define("COPYRIGHT_HOLDER", "Project-Rain owner.");
define("SYS_ADMIN_EMAIL", "contact@project-rain.co.uk");
define("UOTD_ADMIN_EMAIL", "contact@project-rain.co.uk");
    // who gets user of the day pool running low e-mails?
//define("MEMCACHE_SERVERS", "127.0.0.1:11211");

// Email addresses separated by pipe ( | ) that will receive user reported 
// offensive forum posts.
define("POST_REPORT_EMAILS", "contact@project-rain.co.uk");

// Enables PHP error messages in public Web RPCs
define("RPC_DEBUG", false);

// If you use PHPMailer, uncomment the following
// and complete the function definition based on your SMTP server
// (not all fields may be needed)
//
if (0) {
function make_php_mailer() {
    $mail = new PHPMailer();
    $mail->IsSMTP();
    //$mail->Mailer = 'smtp';
    $mail->SMTPAuth = true;
    $mail->SMTPSecure = "tls";
    $mail->Host = "smtp.gmail.com";
    $mail->Port = 587;
    $mail->Username = "contact@project-rain.co.uk";
    $mail->Password = "xxx";
        // Google's application-specific password,
        // if you are using the 2-Step Verification: 16 characters, no spaces.
        // OR: Put here the regular Gmail password if you are not using the
        // 2-Step Verification with your Gmail account.
        // See https://support.google.com/accounts/answer/185833?hl=en";
    $mail->SetFrom('contact@project-rain.co.uk', 'Project Admin');
    $mail->AddReplyTo("contact@project-rain.co.uk", "Project Admin");
    $mail->From = "contact@project-rain.co.uk";
    return $mail;
}
}

function project_banner($title, $prefix) {
    // Put your project title and logo here
    // If you include any links, prepend URL with $prefix

    echo "
    <div class=\"container\">
        <div class=\"row\">
            <div class=\"col-xs-12 page_title\"><img src=\"img/project-rain.png\" alt=\"Project-Rain\" /><br/>$title</div>
        </div>
    </div>
    ";
}

function project_footer($show_return, $show_date, $prefix) {
    // If you include any links, prepend URL with $prefix
    echo "<div class=\"container\">
            <div class=\"row\">
                <div class=\"col-xs-12\">";
    if ($show_return) {
        echo "<a href=".$prefix."index.php>".tra("Main page")."</a> &middot; <a href=".$prefix."home.php>".tra("Your account")."</a><br/>\n";
    }
    if ($show_date) {
        $t = time_str(time());
        echo "<p class=\"text-muted\">".tra("Generated")." $t</p>\n";
    }
    echo "</div>
    </div>";
    echo "<div class=\"row\">
            <div class=\"col-xs-6\" style=\"text-align:center;\">
                <a href=\"https://boinc.berkeley.edu/\"><img align=\"middle\" border=\"0\" src=\"img/pb_boinc.gif\" alt=\"Powered by BOINC\"></a>
            </div>
            <div class=\"col-xs-6\" style=\"text-align:center;\">
                <a href=\"https://www.digitalocean.com/?refcode=b86d3e55f889\"><img src=\"img/DigitalOcean.png\" alt=\"DigitalOcean\" style=\"padding-top:10px;\"/></a>
            </div>
        </div>
    </div>";
}

// Authorize access to administrative pages.
// You can check for login, IP address, or whatever you want.
//
function auth_ops_example() {
    // if running from cmdline, skip checks
    //
    if (!$_SERVER['REMOTE_ADDR']) {
        return;
    }

    // example: require login as a particular user (id 1 in this case)
    //
    if (0) {
        auth_ops_userid(array(1));
        return;
    }

    // example: require that logged-in user have ADMIN or DEV flags
    // set in their forum_prefs.privilege
    //
    if (0) {
        auth_ops_privilege();
        return;
    }
}

function show_profile_heading1() {
    return tra("Your personal background.");
}

function show_profile_question1() {
    return tra("Tell us about yourself. You could tell us where you're from, your age, occupation, hobbies, or anything else about yourself.");
}

function show_profile_heading2() {
    return tra("Your opinions about %1", PROJECT);
}

function show_profile_question2() {
    return tra("Tell us your thoughts about %1<ol>
    <li>Why do you run %1?
    <li>What are your views about the project?
    <li>Any suggestions?
    </ol>", PROJECT);
}

function project_workunit($wu){
    // shown in the workunit page
}

function project_user_summary($user){
    // shown in the user summary page
}

function project_user_page_private($user){
    // shown in the private account page
}

if (0) {

// show project-specific credit on user/team pages
//
function show_app_credit_user($user, $app_name, $appids) {
    $t = 0;
    $a = 0;
    $n = 0;
    foreach ($appids as $appid) {
        $cu = BoincCreditUser::lookup(
            "userid=$user->id and appid=$appid and credit_type=0"
        );
        if ($cu) {
            $t += $cu->total;
            $a += $cu->expavg;
            $n += $cu->njobs;
        }
    }
    row2("$app_name credit",
        format_credit_large($t)." total, ".
        format_credit($a)." average".
        " ($n tasks)"
    );
}
function show_app_credit_team($team, $app_name, $appids) {
    $t = 0;
    $a = 0;
    $n = 0;
    foreach ($appids as $appid) {
        $ct = BoincCreditTeam::lookup(
            "teamid=$team->id and appid=$appid and credit_type=0"
        );
        if ($ct) {
            $t += $ct->total;
            $a += $ct->expavg;
            $n += $ct->njobs;
        }
    }
    row2("$app_name credit",
        format_credit_large($t)." total, ".
        format_credit($a)." average".
        " ($n tasks)"
    );
}

// a list of "sub-projects", used in the display of per-app credit and badges.
// A subproject is:
// - a set of 1 or more apps; an app can belong to at most 1 subproject.
// - a name, shown on the web site
// - a short name, used in badge names.  Don't use "total".
//
$sub_projects = array(
    array("name" => "Remote Test", "short_name" => "RT", "appids" => array(16)),
    array("name" => "Uppercase", "short_name" => "UC", "appids" => array(1, 25)),
);

function project_user_credit($user){
    global $sub_projects;
    foreach ($sub_projects as $sp) {
        show_app_credit_user($user, $sp["name"], $sp["appids"]);
    }
}

function project_team_credit($team) {
    global $sub_projects;
    foreach ($sub_projects as $sp) {
        show_app_credit_team($team, $sp["name"], $sp["appids"]);
    }
}

}   // if(0)

function project_forum_post_rules() {
    // additional forum post rules
    return "";
}

?>
