<?php

require_once("../project/project.inc");
require_once("../inc/countries.inc");
require_once("../inc/db.inc");

function db_init() {
    if (project_is_stopped()) {
        echo "Project is shut down for maintenance - please try again later\n";
        exit();
    }
    $retval = db_init_aux();
    if ($retval == 1) {
        echo "Unable to connect to database - please try again later\n";
		echo "Error: ", mysql_errno(), mysql_error();
        exit();
    }
    if ($retval == 2) {
        echo "Unable to select database - please try again later";
		echo mysql_error();
        exit();
    }
}

// Sends the authenticator to the given email address
//
function send_auth_email($email_addr, $auth) {
    mail($email_addr, PROJECT." new account confirmation",
"This email confirms the creation of your ".PROJECT." account.

".PROJECT." URL:          ".MASTER_URL."

Your account ID:             $auth\n

Please save this email.
You will need your account ID to log in to the ".PROJECT." web site."
    );
}

function email_sent_message($email_addr) {
    if (defined('EMAIL_FROM')) {
        $email_from = EMAIL_FROM;
    } else {
        $email_from = URL_BASE;
    }

    echo "
        Your account ID has been emailed to $email_addr.
        <p>
        If the email doesn't arrive in a few minutes,
        your ISP may be blocking it as spam.
        In this case please contact your ISP and
        ask them to not block email from $email_from.
    ";
}

// Initializes the session and returns the authenticator
// for the session (if any)
//
function init_session() {
    $url = parse_url(MASTER_URL);
    $path = $url['path'];
    if (strlen($path)) {
        session_set_cookie_params(0, $path);
    }
    session_start();
// NOTE: in PHP 4.1+, s/key_exists/array_key_exists/
    if (array_key_exists('authenticator', $_SESSION)) {
        return $_SESSION["authenticator"];
    } else {
        return NULL;
    }
}

// if not logged in, put up login form and exit
//
function require_login($user) {
    if (!$user) {
        print_login_form();
        exit();
    }
}

function get_user_from_auth($auth) {
    if ($auth) return lookup_user_auth($auth);
    return NULL;
}

function get_user_from_id($id) {
    if ($id) return lookup_user_id($id);
    return NULL;
}

function get_logged_in_user($must_be_logged_in=true) {
    $authenticator = init_session();
    if (!$authenticator) {
        $authenticator = $_COOKIE['auth'];
    }
    $user = get_user_from_auth($authenticator);
    if ($must_be_logged_in) {
        require_login($user);
    }
    return $user;
}

function show_login($user) {
    if ($user) {
        echo "Logged in as %s.\n", $user->name;
        echo "<br><a href=login_form.php>Log in as someone else.</a>\n";
    } else {
        echo "Not logged in";
    }
}

// output a select form item with the given name,
// from a list of newline-delineated items from the text file.
// If $selection is provided, and if it matches one of the entries in the file,
// it will be selected by default.
//
function show_combo_box($name, $filename, $selection=null) {
    if (!file_exists($filename)) {
        echo "ERROR: $filename does not exist!  Cannot create combo box.<br>";
        exit();
    }
    echo "<select name=\"$name\">\n";

    $file = fopen($filename, "r");

    while ($line = trim(fgets($file, 1024))) {
        if ($line == $selection) {
            echo "<option SELECTED value=\"$line\">$line\n";
        } else {
            echo "<option value=\"$line\">$line\n";
        }
    }

    echo "</select>\n";
    fclose($file);
}

function page_head($title) {
    $styleSheet = URL_BASE . STYLESHEET;

    echo "<html><head><title>".strip_tags($title)."</title>
        <link rel=stylesheet type=text/css href=\"$styleSheet\">
        </head>
        <body bgcolor=ffffff>
        ";
    project_banner($title);
}

function page_tail($is_main=false) {
    echo "<br><hr noshade size=1><center>";
    if (!$is_main) {
        echo "<a href=".URL_BASE.">Return to ".PROJECT." main page</a><br>\n";
    }

    // put your copyright notice etc. here

    echo "<br><br>Copyright &copy; 2004 ".COPYRIGHT_HOLDER."</center>\n</body>\n</html>";
}

function db_error_page() {
    page_head("Database error");
    echo "A database error occurred while handling your request.
        <br>Please try again later.
        <br>If the error persists, please submit a
        <a href=bug_report_form.php>problem report</a>.
    ";
    page_tail();
}

function profile_error_page($str) {
    page_head("Profile error");
    echo "$str<br>\n";
    echo "<p>Click your browser's <b>Back</b> button to try again.\n<p>\n";
    page_tail();
}

function date_str($x) {
    if ($x == 0) return "---";
    // return date("g:i A, l M j", $when);
    return strftime("%Y-%m-%d", $x);
}

function time_str($x) {
    if ($x == 0) return "---";
    //return strftime("%j %m-%d %H:%M:%S", $x)";
    return gmdate('j M Y G:i:s', $x) . " UTC";
}

function pretty_time_str($x) {
    return time_str($x);
}

function start_table($extra="width=100%") {
    echo "<table border=1 cellpadding=5 $extra>";
}

function start_table_noborder($width="100%") {
    echo "<table border=0 cellpadding=5 width=$width>";
}

function end_table() {
    echo "</table>\n";
}

function row1($x, $ncols=2) {
    echo "<tr><td class=heading bgcolor=cccccc colspan=$ncols><b>$x</b></td></tr>\n";
}

function row2($x, $y) {
    if ($x=="") $x="<br>";
    if ($y=="") $y="<br>";
    echo "<tr><td class=fieldname bgcolor=eeeeee align=right valign=top>$x</td><td valign=top><b>$y</b></td></tr>\n";
}
function row2_init($x, $y) {
    echo "<tr><td class=fieldname bgcolor=eeeeee width=40% align=right valign=top>$x</td><td valign=top><b>$y\n";
}

function row2_plain($x, $y) {
    echo "<tr><td>$x</td><td>$y</td></tr>\n";
}

function row3($x, $y, $z) {
    echo "<tr><td width=30% valign=top align=right>$x</td><td>$y</td><td>$z</td></tr>\n";
}

function rowify($string) {
    echo "<tr><td>$string</td></tr>";
}

function random_string() {
    return md5(uniqid(rand(), true));
}

function print_login_form_aux($next_url, $user) {
    echo "
        <form name=f method=post action=login_action.php>
        <input type=hidden name=next_url value='$next_url'>
    ";
    start_table();
    row1("Log in");
    row2("Your account ID:
        <br><font size=-2>
        If you don't know your account ID,
        <a href=get_passwd.php>click here</a>.
        </font>",
        "<input name=authenticator size=40>"
    );
    row2("Remember account ID on this computer",
        "<input type=checkbox name=send_cookie>"
    );
    row2("",
        "<input type=submit value='Log in'>"
    );
    if ($user) {
        row1("Log out");
        row2("You are logged in as $user->name",
            "<a href=logout.php>Log out</a>"
        );
    }
    end_table();
    echo "
        </form>
        <script>
            document.f.authenticator.focus();
        </script>
    ";
}

function print_login_form() {
    page_head("Please log in");
    echo "
        This function requires that you log in.
    ";
    $next_url = $_SERVER['REQUEST_URI'];
    print_login_form_aux($next_url, null);
    page_tail();
}

// Look for an element in a line of XML text
// If it's a single-tag element, and it's present, just return the tag
//
function parse_element($xml, $tag) {
    $element = null;
    $x = strstr($xml, $tag);
    if ($x) {
        if (strstr($tag, "/>")) return $tag;
        $y = substr($x, strlen($tag));
        $n = strpos($y, "<");
        if ($n) {
            $element = substr($y, 0, $n);
        }
    }
    return $element;
}

if (!function_exists("file_get_contents")) {
function file_get_contents($path) {
    $x = "";
    $f = fopen($path, "r");
    if ($f) {
        while (!feof($f)) $x .= fread($f, 4096);
        fclose($f);
    }
    return $x;
}
}

$g_config = null;
function get_config() {
    global $g_config;
    if ($g_config == null) {
       $g_config = file_get_contents("../../config.xml");
    }
    return $g_config;
}

// look for a particular element in the ../../config.xml file
//
function parse_config($config, $tag) {
    $element = parse_element($config, $tag);
    return trim($element);
}

// Call this if for dynamic pages
//
function no_cache() {
    header ("Expires: Mon, 26 Jul 1997 05:00:00 GMT");    // Date in the past
    header ("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT"); // always modified
    header ("Cache-Control: no-cache, must-revalidate");  // HTTP/1.1
    header ("Pragma: no-cache");                          // HTTP/1.0
}

// a valid email address is of the form A@B.C
// where A, B, C are nonempty,
// A and B don't contain @ or .,
// and C doesn't contain @
//
function is_valid_email_addr($addr) {
    $x = strstr($addr, "@");
    if (!$x) return false;
    if (strlen($x) == strlen($addr)) return false;
    $x = substr($x, 1);
    if (strstr($x, "@")) return false;
    $y = strstr($x, ".");
    if (!$y) return false;
    if (strlen($y) == strlen($x)) return false;
    if (strlen($y) == 1) return false;
    return true;
}

// A few functions relating to email-address munging
// A "munged" email address is of the form @X_Y,
// where X is a valid email address
// and Y is a random string not containing _.
// When an email address hasn't been validated yet, it's munged.
// (Used during account creation and email address changes)

function munge_email_addr($email, $string) {
    return "@".$email."_".$string;
}

// if email_addr is of the form @X_Y, split out the X and return true.
// otherwise return false
//
function split_munged_email_addr($addr, $auth, &$email) {
    if (substr($addr, 0, 1) != "@") return false;
    $x = strrchr($addr, "_");
    if (!$x) return false;
    $y = substr($x, 1);
    if ($auth && $y != $auth) return false;
    $email = substr($addr, 1, strlen($addr)-strlen($x)-1);
    return true;
}

// Generates a standard set of links between associated multi-page documents.
//  All linked files must be of the form "$filename_<page number>.html".

function write_page_links($filename, $currPageNum, $numPages) {
    echo "<p>Page $currPageNum of $numPages</p>";

    $nextPageNum = $currPageNum + 1;
    $prevPageNum = $currPageNum - 1;

    // Make the 'previous' and 'next' page links as appropriate.
    if ($currPageNum > 1) {
        echo "<a href={$filename}_{$prevPageNum}.html>Previous Page</a>";

        if ($currPageNum != $numPages) {
            echo " | ";
        }
    }
    if ($currPageNum != $numPages) {
        //fwrite($descriptor, "<a href=$filename" . "_" . $nextPageNum . ".html>Next Page</a>");
        echo "<a href={$filename}_{$nextPageNum}.html>Next Page</a>";
    }

    //fwrite($descriptor, "<p>Jump to Page:\n");
    echo "<p>Jump to Page:\n";

    // Make the individual page links (or a bold non-link for the current page).
    //
    for ($i = 1; $i <= $numPages; $i++) {
        if ($i != $currPageNum) {
            //fwrite($descriptor, "<a href=$filename" . "_" . $i . ".html>$i</a>\n");
            echo "<a href={$filename}_{$i}.html>$i</a>\n";
        } else {
            //fwrite($descriptor, "<b>$i</b>\n");
            echo "<b>$i</b>\n";
        }
    }

}

// Generates a legal filename from a parameter string.

function get_legal_filename($name) {
    $name = ereg_replace(',', '', $name);
    return ereg_replace(' ', '_', $name);
}

// Returns a string containing as many words
// (being collections of characters separated by the character $delimiter)
// as possible such that the total string length is <= $chars characters long.
// If $ellipsis is true, then an ellipsis is added to any sentence which
// is cut short.

function sub_sentence($sentence, $delimiter, $max_chars, $ellipsis=false) {
    $words = explode($delimiter, $sentence);
    $total_chars = 0;
    $count = 0;
    $result = '';

    do {
        if ($count > 0) {
            $result = $result . ' ' . $words[$count];
        } else {
            $result = $result . $words[$count];
        }
        $total_chars += strlen($words[$count]) + 1;
        $count++;
    } while ($count < count($words) && ($total_chars + strlen($words[$count])) <= $max_chars);

    if ($ellipsis && ($count < count($words))) {
        $result = $result . '...';
    }

    return $result;
}

function format_credit($cobblestones) {
    return sprintf("%.2f", $cobblestones);
}

function project_is_stopped() {
    if (file_exists("../../stop_web")) return true;
    return false;
}

function user_links($user) {
    $x = "<a href=show_user.php?userid=$user->id>$user->name</a>";
    if ($user->has_profile) {
        $x .= " <a href=view_profile.php?userid=$user->id><img border=0 src=head_20.png></a>";
    }
    return $x;
}

function host_link($hostid) {
    if ($hostid) {
        return "<a href=show_host_detail.php?hostid=$hostid>$hostid</a>";
    } else {
        return "---";
    }
}

function open_output_buffer() {
    ob_start();
    ob_implicit_flush(0);
}

function close_output_buffer($filename) {
    $fh = fopen($filename, "w");
    $page = ob_get_contents();
    ob_end_clean();
    fwrite($fh, $page);
    fclose($fh);
}


function intelligently_scale_image($sourcefile, $fw, $fh)
{

	//In order to use the default built-in GD library all imageCreateTrueColor
	//has been commented out.
	//If you whish to enable the advanced GD features (resamples copy for instance)
	//simply switch back to the commented functions.
	
list($ow, $oh, $from_type) = getimagesize($sourcefile);

switch($from_type)
{
case 1: // GIF
$srcImage = imageCreateFromGif($sourcefile);
break;
case 2: // JPG
$srcImage = imageCreateFromJpeg($sourcefile);
break;
case 3: // PNG
$srcImage = imageCreateFromPng($sourcefile);
break;
}

$tempw = $fw;
$temph = number_format((($oh*$fw)/$ow), 0);

if($temph < $fh)
{
$tempw = number_format((($ow*$fh)/$oh), 0);
$temph = $fh;
}

//$tempImage = imageCreateTrueColor($tempw, $temph);
//imageAntiAlias($tempImage, true);
$tempImage = imageCreate($tempw, $temph);

//imagecopyresampled($tempImage, $srcImage, 0, 0, 0, 0, $tempw, $temph, $ow, $oh);
imagecopyresized($tempImage, $srcImage, 0, 0, 0, 0, $tempw, $temph, $ow, $oh);


// Calculate offsets
if($temph > $fh)
{
$offsety = number_format(($temph/2)-($fh/2), 0);
$offsetx = 0;
}
else
{
$offsety = 0;
$offsetx = number_format(($tempw/2)-($fw/2), 0);
}

//$destImage = imageCreateTrueColor($fw, $fh);
//imagecopyresampled($destImage, $tempImage, 0, 0, $offsetx, $offsety, $fw, $fh, $fw, $fh);

$destImage = imageCreate($fw, $fh);
imagecopyresized($destImage, $tempImage, 0, 0, $offsetx, $offsety, $fw, $fh, $fw, $fh);

return $destImage; //imageJpeg($destImage, $destfile, $jpegquality);

}

function intelligently_scale_image($sourcefile, $fw, $fh) {

	//In order to use the default built-in GD library all imageCreateTrueColor
	//has been commented out.
	//If you whish to enable the advanced GD features (resamples copy for instance)
	//simply switch back to the commented functions.
	
    list($ow, $oh, $from_type) = getimagesize($sourcefile);

    switch($from_type) {
    case 1: // GIF
        $srcImage = imageCreateFromGif($sourcefile);
        break;
    case 2: // JPG
        $srcImage = imageCreateFromJpeg($sourcefile);
        break;
    case 3: // PNG
        $srcImage = imageCreateFromPng($sourcefile);
        break;
    }

    $tempw = $fw;
    $temph = number_format((($oh*$fw)/$ow), 0);

    if($temph < $fh) {
        $tempw = number_format((($ow*$fh)/$oh), 0);
        $temph = $fh;
    }

//  $tempImage = imageCreateTrueColor($tempw, $temph);
//  imageAntiAlias($tempImage, true);
    $tempImage = imageCreate($tempw, $temph);

//  imagecopyresampled($tempImage, $srcImage, 0, 0, 0, 0, $tempw, $temph, $ow, $oh);
    imagecopyresized($tempImage, $srcImage, 0, 0, 0, 0, $tempw, $temph, $ow, $oh);


    // Calculate offsets
    if($temph $fh) {
        $offsety = number_format(($temph/2)-($fh/2), 0);
        $offsetx = 0;
    } else {
        $offsety = 0;
        $offsetx = number_format(($tempw/2)-($fw/2), 0);
    }

//  $destImage = imageCreateTrueColor($fw, $fh);
//  imagecopyresampled($destImage, $tempImage, 0, 0, $offsetx, $offsety, $fw, $fh, $fw, $fh);

    $destImage = imageCreate($fw, $fh);
    imagecopyresized($destImage, $tempImage, 0, 0, $offsetx, $offsety, $fw, $fh, $fw, $fh);

    return $destImage; //imageJpeg($destImage, $destfile, $jpegquality);
}
?>
