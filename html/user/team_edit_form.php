<?php

require_once("util.inc");
require_once("team.inc");

$authenticator = init_session();
db_init();

    $query = "select * from team where id = $id";
    $result = mysql_query($query);
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
    }
    $team_name = $team->name;
    $team_id = $team->id;
    $team_name_html = $team->name_html;
    $team_url = $team->url;
    $team_description = $team->description;
    $team_type = $team->type;
    page_head("Edit $team_name");
    echo "<table width=780>";
    echo "<tr><td>";
    echo "<form method=post action=team_edit_action.php>";
    echo "<input type=hidden name=id value=$team_id>";
    echo "</td></tr></table>";
    echo "<table><tr>";
    echo "<td>Team name (plain-text version):<br><br>&nbsp;</td>";
    echo "<td><input name=name size=50 value=$team_name>";
    echo "<br><font size=2>This name will be print as-is";
    echo "<br>and is the name you should use when searching for your team.";
    echo "</td></tr></tr>";
    echo "<td>Team name (HTML version):<br><br>&nbsp;</td>";
    echo "<td><input name=name_html size=50 value='$team_name_html'>";
    echo "<br><font size=2>This name will be printed as HTML source, so you may include any HTML";
    echo "<br>code that you want. This will only be displayed in your team's page.";
    echo "<br>If you don't know HTML, just leave this box blank.";
    echo "</td></tr><tr>";
    echo "<td>URL of team web page, if any:<br><font size=2>(without &quot;http://&quot;)</td>";
    echo "<td><input name=url size=60 value=$team_url>";
    echo "<br><font size=2>This page will be linked to from the project's team page.";
    echo "</td></tr><tr>";
    echo "<td valign=top>Description of team:</td>";
    echo "<td><textarea name=description value=$team_description cols=60 rows=10>$team_description</textarea>";
    echo "</td></tr><tr>";
    echo "<td valign=top>Type of team:</td><td>";
    printf("<input type=radio name=type value=4%s>Club<br>", ($team->type==4)?" checked":"");
    printf("<input type=radio name=type value=1%s>Small Company (< 50 employees)<br>", ($team->type==1)?" checked":"");
    printf("<input type=radio name=type value=2%s>Medium Company (50-1000 employees)<br>", ($team->type==2)?" checked":"");
    printf("<input type=radio name=type value=3%s>Large Company (> 1000 employees)<br>", ($team->type==3)?" checked":"");
    printf("<input type=radio name=type value=5%s>Primary School<br>", ($team->type==5)?" checked":"");
    printf("<input type=radio name=type value=6%s>Secondary School<br>", ($team->type==6)?" checked":"");
    printf("<input type=radio name=type value=8%s>Junior College<br>", ($team->type==8)?" checked":"");
    printf("<input type=radio name=type value=7%s>University or Department<br>", ($team->type==7)?" checked":"");
    printf("<input type=radio name=type value=9%s>Government Agency</td>", ($team->type==9)?" checked":"");
    echo "</tr></table>";
    echo "<input type=submit value=\"Edit Team\">";
    echo "</form>";
    page_tail();
?>
