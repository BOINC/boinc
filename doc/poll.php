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

$choices = array($choice0, $choice1, $choice2, $choice3);
generate_functions($choices);
show_overall_choices($choices);

list_item2(
    "How could BOINC be improved?",
    "<textarea name=improved lines=4 cols=60></textarea>"
);
list_bar("Choosing projects");
echo "
    <tr>
        <td bgcolor=$light_blue valign=top>
        Which are the most important factors when you decide
        whether to participate in a BOINC project?
        </td>
        <td>
        <font size=-2>[check all that apply]</font><br>
";
foreach ($factors as $factor) {
    $name = $factor["name"];
    $text = $factor["text"];
    echo "
        <input type=checkbox name=$name> $text <br>\n
    ";
}

echo "
    Other: <input size=50>
        </td></tr>
";

mult_choice($where_item["name"], $where_item["text"], $where_item["choices"]);
list_bar("Your computers");
foreach($comp_items as $item) {
    mult_choice($item["name"], $item["text"], $item["choices"]);
}
list_bar("You");
foreach($you_items as $item) {
    mult_choice($item["name"], $item["text"], $item["choices"]);
}

echo "
    <tr>
        <td width=50% bgcolor=$light_blue valign=top>Nationality</td>
        <td><select name=country>
";
print_country_select();
echo "</select></td></tr>";

list_item("<br>", "When done click: <input type=submit value=OK>");
list_end();
echo "
    </form>
";

?>
