<?php

    require_once("project_specific/project.inc");

// Sends the authenticator to the given email address
//
function send_auth_email($email_addr, $auth) {
    mail($email_addr, PROJECT." new account confirmation",
"This email confirms the creation of your ".PROJECT." account.

".PROJECT." URL:          ".MASTER_URL."

Your account key:            $auth\n

Please save this email.
You will need your account key to log in to the ".PROJECT." web site."
    );
}

// Initializes the session and returns the authenticator
// for the session (if any)
//
function init_session() {
    session_start();
    if (key_exists('authenticator', $_SESSION)) {
        return $_SESSION["authenticator"];
    } else {
        return NULL;
    }
}

// if not logged in, put up login form and exit
//
function require_login($user, $pathMod="") {
    if (!$user) {
        print_login_form($pathMod);
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

function get_profile_from_userid($userid) {
    if ($userid) {
        $result = mysql_query("SELECT * FROM profile WHERE userid = $userid");
        if ($result) {
            return mysql_fetch_assoc($result);
        }
    }
    return null;
}

// $pathMod is a prefix for the path of the login processing form.  This
// allows scripts in other directories (i.e. forum/) to call this function.

function get_logged_in_user($must_be_logged_in=true, $pathMod="") {
    $authenticator = init_session();
    $user = get_user_from_auth($authenticator);
    if ($must_be_logged_in) {
        require_login($user, $pathMod);
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

// write to the given fd if non-null; else echo
//
function write_fd($fd, $str) {
    if ($fd) {
        fwrite($fd, $str);
    } else {
        echo $str;
    }
}

function page_head($title, $user=null, $fd=null, $styleSheet=null) {
    if ($user == NULL) {
        $user = get_logged_in_user(false);
    }

    if ($styleSheet == NULL) {
        $styleSheet = URL_BASE . 'black.css';
    }

    write_fd($fd, "<html><head><title>$title</title>\n");
    if ($styleSheet) {
        write_fd($fd, "<link rel=\"stylesheet\" type=\"text/css\" href=\"$styleSheet\">");
    }
    write_fd($fd, "</head>\n<body bgcolor=ffffff>\n");
    project_banner($user, $fd);
}

function page_tail($fd=null) {
    write_fd($fd, "<br><hr><center><a href=".MASTER_URL.">Return to ".PROJECT." main page</a><br>\n");

    // put your copyright notice etc. here

    write_fd($fd, "<br><br>Copyright (c) 2003 ".PROJECT."</center>\n</body>\n</html>");
}

function db_error_page() {
    page_head("Database error");
    echo "<h2>Database error</h2>
        A database error occurred while handling your request.
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
    // return strftime("%T %b %e, %Y", $x);
    return strftime("%Y-%m-%d %H:%M:%S", $x);
}

function pretty_time_str($x) {
    if ($x == 0) return "&nbsp;";
    return gmdate('D M j, Y g:i a', $x) . " UTC";
}

function start_table() {
    echo "<table border=1 cellpadding=5>";
}

function start_table_noborder($width=640) {
    echo "<table border=0 cellpadding=0 width=$width>";
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
    echo "<tr><td class=fieldname bgcolor=eeeeee width=50% align=right valign=top>$x</td><td valign=top><b>$y</b></td></tr>\n";
}
function row2_init($x, $y) {
    echo "<tr><td class=fieldname bgcolor=eeeeee width=50% align=right valign=top>$x</td><td valign=top><b>$y\n";
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
    return md5(uniqid(rand()));
}

// TODO: we should write this funciton in php --quarl
function print_country_select($country="None") {
    $x = posix_getcwd();
    PassThru("LD_LIBRARY_PATH=/usr/local/gcc/lib $x/country_select '$country'");
}

function print_login_form_aux($next_url, $user, $pathMod='') {
    echo "<form method=post action=\"", $pathMod, "login_action.php\">
        <input type=hidden name=next_url value='$next_url'>
    ";
    start_table();
    row1("Log in");
    row2("Your account key:
        <br><font size=-1>
        If you don't know your account key,
        <a href=get_passwd.php>click here</a>.
        </font>",
        "<input name=authenticator size=40>"
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
}

function print_login_form($pathMod="") {
    page_head("Please log in");
    echo "
        <h3>Please log in</h3>
        This function requires that you log in.
    ";
    $next_url = $_SERVER['REQUEST_URI'];
    print_login_form_aux($next_url, null, $pathMod);
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

// look for a particular element in the ../config.xml file
//
function parse_config($tag, $pathMod='') {
    $element = null;
    $fp = fopen($pathMod . "../config.xml", "r");
    while (1) {
        $buf = fgets($fp, 1024);
        if ($buf == null) break;
        $element = parse_element($buf, $tag);
        if ($element) break;
    }
    fclose($fp);
    return $element;
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
function split_munged_email_addr($addr, $string, &$email) {
    if (substr($addr, 0, 1) != "@") return false;
    $x = strrchr($addr, "_");
    if (!$x) return false;
    $y = substr($x, 1);
    if ($y != $string) return false;
    $email = substr($addr, 1, strlen($addr)-strlen($x)-1);
    return true;
}

// Generates a standard set of links between associated multi-page documents.
//  All linked files must be of the form "$filename_<page number>.html".

function write_page_links($filename, $currPageNum, $numPages, $descriptor) {
  fwrite($descriptor, "<p>Page $currPageNum of $numPages</p>");

  $nextPageNum = $currPageNum + 1;
  $prevPageNum = $currPageNum - 1;

  // Make the 'previous' and 'next' page links as appropriate.
  if ($currPageNum > 1) {
    fwrite($descriptor, "<a href=$filename" . "_" . $prevPageNum . ".html>Previous Page</a>");

    if ($currPageNum != $numPages) {
      fwrite($descriptor, " | ");
    }
  }
  if ($currPageNum != $numPages) {
    fwrite($descriptor, "<a href=$filename" . "_" . $nextPageNum . ".html>Next Page</a>");
  }

  fwrite($descriptor, "<p><table border=0>\n<tr>\n<td>Jump to Page:</td>\n");

  // Make the individual page links (or a bold non-link for the current page).
  for ($i = 1; $i <= $numPages; $i++) {
    if ($i != $currPageNum) {
      fwrite($descriptor, "<td><a href=$filename" . "_" . $i . ".html>$i</a></td>\n");
    } else {
      fwrite($descriptor, "<td><b>$i</b></td>\n");
    }
  }

  fwrite($descriptor, "</table>\n");
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

// TODO: Generalize this.  Currently it is just pasted from error.inc

function doError($msg, $info = '') {
    page_head('Error');
    echo '<p class="title">'.$msg.'</p>';
    echo "<p>The error has been recorded,
        and the website administrator will investigate the issue
        as soon as possible.
        We sincerely apologize for the inconvenience.</p>
    ";
    page_tail();
    exit();
}
?>
