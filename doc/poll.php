<?php

require_once("docutil.php");
require_once("countries.inc");
require_once("translation.inc");
require_once("poll.inc");
require_once("poll_data.inc");

page_head(tr(POLL_TITLE));

echo "
<body onload=\"disable_all();\">
<p>
".tr(POLL_INTRO)."
";

echo "<form name=blah action=poll_action.php>\n";
list_start();
list_bar(tr(POLL_RUN));

generate_functions($overall_choices);
show_choices($overall_choices, $run_boinc);

list_bar(tr(POLL_PARTICIPATION));
show_choices($project_items, "");
list_bar(tr(POLL_COMPUTERS));
show_choices($comp_items, "");
list_bar(tr(POLL_YOU));
show_choices($you_items, "");

echo "
    <tr>
        <td class=fieldname valign=top>".tr(POLL_NATIONALITY)."</td>
        <td><select name=$country>
";
echo country_select();
echo "</select></td></tr>";
list_bar(tr(POLL_COMMENTS));
list_item2(
    tr(POLL_COMMENTS_QUESTION),
    "<textarea name=$improved rows=10 cols=60></textarea>"
);

list_item("<br>", tr(POLL_DONE)."<input type=submit value=OK>");
list_end();
echo "
    </form>
";
page_tail(true);

?>
