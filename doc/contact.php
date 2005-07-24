<?php
require_once("docutil.php");
page_head("Personnel and contributors");
echo "

The BOINC project is based at the
<a href=http://ssl.berkeley.edu>Space Sciences Laboratory</a>
at the University of California, Berkeley.
Project staff include:
<dl>
<dt> <b>Dr. David P. Anderson</b>
<dd>
Director and architect.  Contact him at davea at ssl.berkeley.edu.

<dt><b>Rom Walton</b>
<dd>
Developer (mostly Windows).
Contact him at rwalton at ssl.berkeley.edu.

<dt><b>Jeff Cobb</b>
<dd>Developer and system administrator
<dt><b>Matt Lebofsky</b>
<dd>Developer and system administrator
<dt><b>Court Cannick</b>
<dd>System administrator
</dl>

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
show_name("Tolu Aina");
show_name("Bruce Allen");
show_name("David Anderson");
show_name("Noaa Avital");
show_name("Don Bashford");
show_name("Lars Bausch");
show_name("Brian Boshes");
show_name("Jens Breitbart");
show_name("Tim Brown");
show_name("Karl Chen");
show_name("Pietro Cicotti");
show_name("Seth Cooper");
show_name("Markku Degerholm");
show_name("Glenn Dill");
show_name("James Drews");
show_name("Charlie Fenton");
show_name("John Flynn III");
show_name("Michael Gary");
show_name("Gary Gibson");
show_name("Walt Gribben");
show_name("Jim Harris");
show_name("Volker Hatzenberger");
show_name("Ian Hay");
show_name("Eric Heien");
show_name("Thomas Horsten");
show_name("Daniel Hsu");
show_name("Takafumi Kawana");
show_name("John Keck");
show_name("John Kirby");
show_name("Eric Korpela");
show_name("Janus Kristensen");
show_name("Tim Lan");
show_name("Gilson Laurent");
show_name("Bernd Machenschalk");
show_name("Sebastian Masch");
show_name("John McLeod VII");
show_name("Kenichi Miyoshi");
show_name("Tony Murray");
show_name("Eric Myers");
show_name("Kjell Nedrelid");
show_name("Rob Ogilvie");
show_name("J.R. Oldroyd");
show_name("Jakob Pedersen");
show_name("Stephen Pellicer");
show_name("Reinhard Prix");
show_name("Andy Read");
show_name("Kevin Reed");
show_name("Thomas Richard");
show_name("Nikolay Saharov");
show_name("Jens Seidler");
show_name("Peter Smithson");
show_name("Christian S&oslash'ttrup");
show_name("Michela Taufer");
show_name("Mathias Walter");
show_name("Rom Walton");
show_name("Oliver Wang");
show_name("Frank Weiler");
echo "
</table>
<p>
BOINC testers include:
<p>
";
$i = 0;
echo "<table bgcolor=ddddff width=100% border=2 cellpadding=6>\n";
show_name("Comatose");
show_name("Jay Curtis");
show_name("Fons Maartens");
show_name("Rob Hague");
show_name("Fred Harmon aka fharmon");
show_name("Darrell Holz");
show_name("John McLeod VII");
show_name("Dennis Peters");
show_name("Scott Sutherland");
show_name("Chris Sutton");
echo "
</table>
<p>
Many thanks to Komori Hitoshi for proof-reading this web site.
";
page_tail();
?>
