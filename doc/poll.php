<?php

require_once("docutil.php");
require_once("countries.inc");
require_once("../html/inc/translation.inc");
require_once("poll.inc");
require_once("poll_data.inc");

page_head(tra("BOINC user survey"));

echo "The BOINC poll is closed.  See results
    <a href=poll_results.php>here</a>.";
page_tail();
exit;

echo "
<body onload=\"disable_all();\">
<p>
".tra("Several volunteer computing projects, including Climateprediction.net, Einstein@home, and SETI@home, use software called BOINC.  If you participate in projects like this, we request that you answer the following questions.  This will help BOINC-based projects increase participation and achieve greater scientific results. <p> Please answer as many questions as you want, then go to the bottom and click OK. If you previously completed the survey but your answers have changed, please complete it again - your new answers will replace the old ones. <p> The current results of the survey are <a href=poll_results.php>here</a>. "
)."
";

echo "<form name=blah action=poll_action.php>\n";
list_start();
list_bar(tra("Do you run BOINC?"));

generate_functions($overall_choices);
show_choices($overall_choices, $run_boinc);

list_bar(tra("Your participation"));
show_choices($project_items, "");
list_bar(tra("Your computers"));
show_choices($comp_items, "");
list_bar(tra("You"));
show_choices($you_items, "");

echo "
    <tr>
        <td class=fieldname valign=top>".tra("Nationality")."</td>
        <td><select name=$country>
";
echo country_select();
echo "</select></td></tr>";
list_bar(tra("Comments"));
list_item2(
    tra("Please suggest ways that BOINC, and the projects that use it, could be improved:"),
    "<textarea name=$improved rows=10 cols=60></textarea>"
);

list_item("<br>", tra("When done click:")."<input type=submit value=OK>");
list_end();
echo "
    </form>
";
page_tail(true);

?>
