<?php
require_once("docutil.php");
page_head("Acknowledgements");
echo "

The BOINC project is based at the Space Sciences Laboratory
at the University of California, Berkeley.

<p>
BOINC developers (many of them volunteers) include:
<p>
";
$i = 0;
$n = 3;
function show_name($x) {
    global $i;
    global $n;
    if ($i == 0) echo "<tr>";
    echo "<td>$x</td>\n";
    $i  = ($i+1)%$n;
}

echo "<table bgcolor=ddddff width=100% border=2 cellpadding=6>\n";
show_name("David Anderson");
show_name("Noaa Avital");
show_name("Brian Boshes");
show_name("Karl Chen");
show_name("Pietro Cicotti");
show_name("Seth Cooper");
show_name("James Drews");
show_name("Michael Gary");
show_name("Gary Gibson");
show_name("Eric Heien");
show_name("Thomas Horsten");
show_name("Daniel Hsu");
show_name("John Keck");
show_name("John Kirby");
show_name("Eric Korpela");
show_name("Janus Kristensen");
show_name("Tim Lan");
show_name("Sebastian Masch");
show_name("Kenichi Miyoshi");
show_name("Stephen Pellicer");
show_name("Jens Seidler");
show_name("Christian Soettrup");
show_name("Michela Taufer");
show_name("Mathias Walter");
show_name("Rom Walton");
show_name("Oliver Wang");
echo "
</table>
<p>
BOINC testers include:
<p>
";
echo "<table bgcolor=ddddff width=100% border=2 cellpadding=6>\n";
show_name("Comatose");
show_name("Jay Curtis");
show_name("Fons Maartens");
show_name("Rob Hague");
show_name("Darrell Holz");
show_name("John McLeod VII");
show_name("Dennis Peters");
show_name("Scott Sutherland");
show_name("Chris Sutton");
echo "
</table>
<p>
Many thanks to Komori Hitoshi for proof-reading this web site.
<p>
The BOINC logo uses the Planet Benson font from
<a href=http://www.larabiefonts.com>Larabie Fonts</a>.
Hi-res logos:
<a href=logo.png>PNG</a>, <a href=logo.jpg>JPEG</a>,
<a href=logo.gif>GIFF</a>.
";
page_tail();
?>
