<?php

require_once("util.inc");
require_once("team.inc");

$authenticator = init_session();
db_init();

$user = get_user_from_auth($authenticator);

if ($user == NULL) {
    print_login_form();
    exit();
}

    page_head("Create a team");
?>

<table width=780>
<tr><td valign=top>
<p>
Use this form to create a team.
You'll become the founding member of the team.
</td></tr></table>
<p>
<form method=post action=team_create_action.php>
<table>
<tr>
<td valign=top>Team name (text version)
<br><font size=2>This name will be printed as text.
It's the name
you should use <br>when searching for your Team.
</td>
<td valign=top><input name=name type=text size=50>
</td>
</tr><tr>
<td width=30%>Team name (HTML version)
<br><font size=2>This name will be shown as HTML,
so you may include any HTML code that you want.
If you don't know HTML, just leave this box blank.
</td>
<td valign=top><input name=name_html type=text size=50>
</td>
</tr><tr>
<td valign=top>URL of team web page, if any:<br><font size=2>(without "http://")
<br><font size=2>This URL will be linked to from the team's page.
</td>
<td valign=top><input name=url size=60>
</td>
</tr><tr>
<td valign=top>Description of team:</td>
<td valign=top><textarea name=description cols=60 rows=10></textarea></td>
</tr><tr>
<td valign=top>Type of team:</td>
<td valign=top>
<input type=radio name=type value=1 checked> Club
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
<input type=radio name=type value=7> Government Agency
</td>
</tr>

<tr><td>Country</td>
<td>
<select name=country>
<?php
print_country_select("None");
?>

</select>
</td></tr>
<tr><td valign=top><br></td><td valign=top>
<input type=submit name=new value="Create Team">
</td></tr>
</table>
</form>

<?php
    page_tail();
?>
