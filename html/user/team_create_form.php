<?php

require_once("util.inc");
require_once("team.inc");

db_init();

$user = get_logged_in_user();

page_head("Create a team");

echo "<form method=post action=team_create_action.php>\n";
start_table();
row1("Create a team");
row2( "Team name (text version)
    <br><font size=2>
    Don't use any HTML tags.
    This name will be used in the searchable team list.",
    "<input name=name type=text size=50>"
);
row2("Team name (HTML version)
    <br><font size=2>
    You may include HTML formatting, link, and image tags.
    If you don't know HTML, just leave this box blank.",
    "<input name=name_html type=text size=50>"
);
row2("URL of team web page, if any:<br><font size=2>(without \"http://\")
    This URL will be linked to from the team's page on this site.",
    "<input name=url size=60>"
);
row2("Description of team:",
    "<textarea name=description cols=60 rows=10></textarea>"
);
row2("Type of team:",
    "<input type=radio name=type value=1 checked> Other
    <br>
    <input type=radio name=type value=2> Company
    <br>
    <input type=radio name=type value=3> Primary School
    <br>
    <input type=radio name=type value=4> Secondary School
    <br>
    <input type=radio name=type value=5> Junior College
    <br>
    <input type=radio name=type value=6> University or Department
    <br>
    <input type=radio name=type value=7> Government Agency"
);
row2_init("Country",
    "<select name=country>"
);
print_country_select();

echo "</select></b></td></tr>\n";
row2("",
    "<input type=submit name=new value=\"Create Team\">"
);
end_table();
echo "</form>\n";

page_tail();
?>
