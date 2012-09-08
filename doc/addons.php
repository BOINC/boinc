<?php
require_once("docutil.php");
require_once("addon_data.php");

$strip_header = $_GET['strip_header'];

if (!$strip_header) {
page_head('BOINC add-on software');
echo "
<p>
The following programs complement or enhance BOINC.
Note:
<ul>
<li>

These applications are not endorsed by BOINC and
you use them at your own risk.
<li>
We do not provide instructions for installing these applications.
However, the author may have provided some help on installing or
uninstalling the application.
If this is not enough you should contact the author.
Instructions for installing and running BOINC are
<a href=http://boinc.berkeley.edu/wiki/User_manual>here</a>.
<li>
To submit an item for inclusion in this list,
please email <a href=http://boinc.berkeley.edu/trac/wiki/ProjectPeople>David Anderson</a>.
</ul>
";

}

function show_group($name, $list, $short_name) {
	echo "
		<h2>$name</h2>
		<table border=1 cellpadding=6 width=100%>
		<tr>
			<th>Add-on<br><font size=-2>click for info</font></th>
			<th>Description</th>
		</tr>
	";
	shuffle($list);
	foreach ($list as $item) {
		$file = $item[0];
		$itemname = $item[1];
		$desc = $item[3];
        $iname = urlencode($file);
		echo "
			<tr><td><a href=http://boinc.berkeley.edu/addon_item.php?platform=$short_name&item=$iname>$itemname</a></td>
				<td>$desc</td>
			</tr>
		";
	}
	echo "</table>
	";
}

echo "<a name=windows></a>\n";
show_group("Windows", $win, 'win');
echo "<a name=linux></a>\n";
show_group("Linux", $linux, 'linux');
echo "<a name=mac></a>\n";
show_group("Mac", $mac, 'mac');
echo "<a name=mobile></a>\n";
show_group("Mobile", $mobile, 'mobile');
echo "<a name=browser></a>\n";
show_group("Web browser toolbars and plugins", $browser, 'browser');
echo "<a name=web></a>\n";
show_group("Web applications", $web, 'web');

if (!$strip_header) {
page_tail();
}
?>
