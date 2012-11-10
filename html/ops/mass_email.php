<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

//   This is a script for sending mass email to project participants.
//   Test it first and use it with care, to avoid alienating your
//   project's volunteers.

//   Note also that the queries such as the one to find lapsed users
//   assume that the project keeps the results in the DB for some interval
//   such as a week, before purging them.  So active users will always
//   have at least one result in the database.

require_once("../inc/util_ops.inc");
require_once("../inc/email.inc");

function exit_error($message) {
    echo "Error: $message <br>";
    exit();
}

// These are set to large values because otherwise the script has
// a tendency to just stop after some time.
//
ini_set ("memory_limit", "20M");
set_time_limit(3600);

$receiver = 0;
$receiver = post_int('receiver', true);
$subject = post_str('subject', true);
$body = post_str('body', true);
$body = stripslashes($body);

admin_page_head("Send mass email");

if ($receiver > 0) {
    db_init();
    switch ($receiver) {
    case 1:
        // all users
        $query = "select * from user where send_email > 0";
        break;
    case 2:
        // unsuccessful users
        $week_ago = time(0) - 7*86400;
        $query = "select user.id,user.name,user.email_addr from user left join result on user.id=result.userid where send_email>0 and total_credit=0 and user.create_time<$week_ago and isnull(result.id)";
        break;
    case 3:
        // successful users
        $query = "select * from user where send_email>0 and total_credit>0";
        break;
    case 4:
        // currently contributing users
        $query = "select distinct user.id,user.name,user.email_addr from user left join result on user.id=result.userid where send_email>0 and !isnull(result.id)";
        break;
    case 5:
        // lapsed users
        $query = "select user.id,user.name,user.email_addr from user left join result on user.id=result.userid where send_email>0 and total_credit>0 and isnull(result.id)";
        break;
    default:
        // should never happen!
        exit_error("Got impossible value of receiver from selection!");
    }
    // FOR DEBUGGING
    $query .= " LIMIT 10";

    $result = mysql_query($query);
    while ($user = mysql_fetch_object($result)) {
    	// TODO: might want to also replace TOTAL_CREDIT, RAC, and similar.
        $body_to_send = str_replace("USERNAME", $user->name, $body);
        $body_to_send .= "\n\nTo opt out of future emails from ".PROJECT.", please edit your project preferences at ".URL_BASE."prefs.php?subset=project\n";
        $retval = send_email($user, $subject, $body_to_send);
        if ($retval) {
            // send_email returns TRUE on success
            echo "Sent email to $user->name [$user->id] at $user->email_addr <br>";
        } else {
            echo "<font color=RED>send_email() to $user->name [$user->id] at $user->email_addr failed with error $retval</font><br>";
        }
        // try to get output on the screen for feedback.  May not help...
        flush();
    }
    exit();
}

echo "<form method=\"post\" action=\"mass_email.php\">\n";
echo "<p>\n";

start_table();
echo "<tr><td align=right>Send email to: </td><td> ";
echo "
    <select name=\"receiver\">
      <option value='0' selected> PLEASE CHOOSE DESIRED SET OF USERS TO EMAIL
      <option value='1' > All users
      <option value='2' > Unsuccessful users: total_credit = 0, create time > 1 week ago, NO results in DB 
      <option value='3' > Successful users: total_credit > 0
      <option value='4' > Currently contributing users: total_credit > 0 and at least one result in DB
      <option value='5' > Lapsed users: total_credit > 0 but NO results in DB
    </select>
    </td></tr>
    <tr>
      <td align=\"right\">Email subject</td>
      <td><input name=\"subject\" size=\"50\"></td>
      </tr>
    <tr>
      <td align=\"right\">Email body (USERNAME will be replaced)</td>
      <td><textarea name=\"body\" rows=25 cols=50 id=\"body\"></textarea></td>
    </tr>
        ";
row2("", "<input type=\"submit\" value=\"OK\">\n");

end_table();
echo "</form>\n";
?>
