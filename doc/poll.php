<?php

require_once("docutil.php");
require_once("countries.inc");
require_once("poll_data.inc");
require_once("poll.inc");

page_head("BOINC user survey");

echo "
<p>
Several volunteer computing projects,
including Climateprediction.net, Einstein@home,
and SETI@home, use software called BOINC.
If you participate in projects like this,
we request that you answer the following questions.
This will help BOINC-based projects increase
participation and achieve greater scientific results.
<p>
Please answer as many questions as you want,
then go to the bottom and click OK.
";

echo "<form name=blah action=poll_action.php>\n";
list_start();
list_bar("Do you run BOINC?");

generate_functions($overall_choices);
show_choices($overall_choices, $run_boinc);

list_bar("Choosing projects");
show_choice($factors, "");

show_choice($where_item, "");
list_bar("Your computers");
show_choices($comp_items, "");
list_bar("You");
show_choices($you_items, "");

echo "
    <tr>
        <td bgcolor=$light_blue valign=top>Nationality</td>
        <td><select name=$country>
";
print_country_select();
echo "</select></td></tr>";
list_bar("Comments");
list_item2(
    "Please suggest ways that BOINC,
    and the projects that use it, could be improved:",
    "<textarea name=$improved lines=4 cols=60></textarea>"
);

list_item("<br>", "When done click: <input type=submit value=OK>");
list_end();
echo "
    </form>
";

?>
