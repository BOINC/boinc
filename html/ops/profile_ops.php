<?php
require_once('../html_user/profile.inc');

define("ACCEPT", "Accept");
define("REJECT", "Reject");
define("SKIP", "Skip");

// TODO: Move these to a better location?
$verification['-1'] = "DENIED";
$verification['0'] = "UNRATED";
$verification['1'] = "APPROVED";

if (array_key_exists('num', $_GET) && array_key_exists('set', $_GET)) {
    $num = $_GET['num'];
    $set = $_GET['set'];

    $sql = "SELECT * FROM profile ";

    switch($set) {
    case 'approved':
        $sql = $sql . "WHERE (uotd_time IS NULL) AND (verification = 1) ";
        break;
    case 'unrated':
        $sql = $sql . "WHERE (uotd_time IS NULL) AND (verification = 0) ORDER BY recommend DESC, reject ASC ";
        break;
    case 'rejected':
        $sql = $sql . "WHERE (uotd_time IS NULL) AND (verification = -1 ) ";
        break;
    case 'uotd':
        $sql = $sql . "WHERE uotd_time IS NOT NULL ORDER BY uotd_time ASC ";
        break;
    case 'all':
    }

    $sql = $sql . "LIMIT " . $num . ", 1";

    $result = mysql_query($sql);
    if (!$result) {
        // TODO: DB error page;
    } else {
        $profile = mysql_fetch_assoc($result);
        if (!$profile) {
            echo "No more profiles in this category.<p><a href=index.php>Return to ", PROJECT, " Project Management</a>";
            exit();
        }

        if (array_key_exists('submit', $_POST)) {
            if ($_POST['submit'] == ACCEPT) {
                $vote = 1;
            } else if ($_POST['submit'] == REJECT) {
                $vote = -1;
            } else if ($_POST['submit'] == SKIP) {
                $vote = 0;
            } else {
                echo "Invalid score.  Please press back and try again.";
            }

            if ($vote == -1 || $vote == 1) {
                $sql = "UPDATE profile SET verification = $vote WHERE userid = " . $profile['userid'];
                mysql_query($sql);
            }
                $header = "Location: profile_ops.php?set=" . $set . "&num=" . ($num+1);
                header($header);
        }

        // TODO: Make this a standard ops page head;
        echo "<html><head><title>$title</title><body bgcolor=ffffff>\n";
        show_verify_tools($num, $profile);

        // Put the profile itself into a table to separate it visually from the controls.
        echo "<br><center><table border = 1><tr><td align=\"center\">";
        show_profile($profile['userid'], true);
        echo "</td></tr></table>";
        echo "<p><a href=index.php>Return to ", PROJECT, " Project Management</a></center>";

    }
    exit();

}

echo "<h2>User of the day validation</h2>";

$result = mysql_query("SELECT * FROM profile WHERE (verification = 1) AND (uotd_time IS NULL)");
$result2 = mysql_query("SELECT * FROM profile WHERE (verification = 0) AND (uotd_time IS NULL)");

if ($result && $result2) {
    echo "<p>There are currently <b>", mysql_numrows($result), "</b> profiles which have been approved for User of the Day; <b>", mysql_num_rows($result2), "</b> unverified potential candidates.</p>";
    mysql_free_result($result);
    mysql_free_result($result2);
} else {
    // DATABASE ERROR PAGE;
}

$result = mysql_query("SELECT *, (recommend - reject) AS score FROM profile WHERE uotd_time IS NULL ORDER BY score DESC");
if ($result && mysql_numrows($result) > 0) {
    echo "<a href=\"", $_SERVER['PHP_SELF'], "?num=0\">Verify additional profiles</a>";
}

function show_verify_tools($num, $profile) {
    global $verification;
    global $set;

    echo "
<table width=\"100%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\" bgcolor=\"#C0C0C0\">
  <tr>
    <td>User ID: <b>", $profile['userid'], "</b><br>
        Last selected UOTD: <b>";

    if ($profile['uotd_time']) {
        echo date("F jS, Y", $profile['uotd_time']);
    } else {
        echo "Never";
    }

    echo "</td>\n<td align=\"center\">", $profile['recommend'], " recommendation(s), ", $profile['reject'], " rejection(s).<br>";

    echo "Current status: ", $verification[$profile['verification']];

    echo"
    </td>\n<td align=\"right\">
      <form action=\"", $_SERVER['PHP_SELF'], "?set=", $set, "&num=", $num, "\" method=\"post\">
        <input type=\"submit\" name=\"submit\" value=\"", ACCEPT, "\">
        <input type=\"submit\" name=\"submit\" value=\"", REJECT, "\">
        <input type=\"submit\" name=\"submit\" value=\"", SKIP, "\">
      </form></td>
  </tr>
</table>
";
}
?>