<?php
require_once("docutil.php");
page_head("Personnel and contributors");
echo "

The BOINC project is based at the
<a href=http://ssl.berkeley.edu>Space Sciences Laboratory</a>
at the University of California, Berkeley.
Project staff are:
<dl>
<dt> <b>Dr. David P. Anderson</b>
<dd>
Director and architect.  Contact him at davea at ssl.berkeley.edu.

<dt><b>Rom Walton</b>
<dd>
Developer.
Contact him at rwalton at ssl.berkeley.edu.

<dt><b>Charlie Fenton</b>
<dd>
Developer.

<dt><b>Walt Gribben</b>
<dd>
Developer.

</dl>

<h2>Development organization</h2>
BOINC development is done by BOINC staff,
by the staff of various BOINC-based projects,
and by volunteer programmers.
Development is divided into several areas.
Each area is managed by an 'owner'.
<p>
<table cellpadding=6 border=1>
<tr><th>Area</th><th>Owner</th><th>Key contributors</th></tr>
<tr>
    <td>API</td>
    <td>Bruce Allen</td>
    <td>Bernd Machenschalk</td>
</tr>
<tr>
    <td>BOINC Manager</td>
    <td>Rom Walton</td>
    <td><br></td>
</tr>
<tr>
    <td>Core client</td>
    <td>David Anderson</td>
    <td>John McLeod, Carl Christensen</td>
</tr>
<tr>
    <td>Mac OS X</td>
    <td>Charlie Fenton</td>
    <td><br></td>
</tr>
<tr>
    <td>Testing and release management</td>
    <td>Rom Walton</td>
    <td>Jens Seidler</td>
</tr>
<tr>
    <td>Server</td>
    <td>Bruce Allen</td>
    <td>David Anderson</td>
</tr>
<tr>
    <td>Translations of GUI and web text</td>
    <td>Jens Seidler</td>
    <td>Janus Kristensen</td>
</tr>
<tr>
    <td>Unix build system</td>
    <td>Reinhard Prix</td>
    <td>Eric Korpela</td>
</tr>
<tr>
    <td>Web features</td>
    <td>Janus Kristensen</td>
    <td><a href=http://rob.axpr.net/>Rob Ogilvie</a></td>
</tr>
<tr>
    <td>Wiki-based documentation</td>
    <td>Christopher Malton and Rytis Slatkevicius</td>
    <td>Paul Buck</td>
</tr>
<tr>
    <td>Windows installer and screensaver</td>
    <td>Rom Walton</td>
    <td><br></td>
</tr>
</table>

<h2>Developers</h2>
The following people have contributed to the development of
the BOINC software:
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
show_name("Jeff Cobb");
show_name("Seth Cooper");
show_name("Carl Christensen");
show_name("Markku Degerholm");
show_name("Glenn Dill");
show_name("James Drews");
show_name("Charlie Fenton");
show_name("John Flynn III");
show_name("Jan Gall");
show_name("Michael Gary");
show_name("Marco Gazzoni");
show_name("Gary Gibson");
show_name("David Goodenough");
show_name("Walt Gribben");
show_name("John F. Hall");
show_name("Jim Harris");
show_name("Volker Hatzenberger");
show_name("Ian Hay");
show_name("Eric Heien");
show_name("Darrell Holz");
show_name("Thomas Horsten");
show_name("Daniel Hsu");
show_name("Takafumi Kawana");
show_name("John Keck");
show_name("David Kim");
show_name("John Kirby");
show_name("Eric Korpela");
show_name("Janus Kristensen");
show_name("Tim Lan");
show_name("Egon Larsson");
show_name("Gilson Laurent");
show_name("Matt Lebofsky");
show_name("Bernd Machenschalk");
show_name("Christopher Malton");
show_name("Sebastian Masch");
show_name("John McLeod VII");
show_name("Kenichi Miyoshi");
show_name("Tony Murray");
show_name("Eric Myers");
show_name("Harold Naparst");
show_name("Kjell Nedrelid");
show_name("Nikos Ntarmos");
show_name("Rob Ogilvie");
show_name("J.R. Oldroyd");
show_name("Jakob Pedersen");
show_name("Stephen Pellicer");
show_name("Reinhard Prix");
show_name("Tetsuji Maverick Rai");
show_name("Andy Read");
show_name("Kevin Reed");
show_name("Thomas Richard");
show_name("Nikolay Saharov");
show_name("Alex A. dos Santos");
show_name("Steven Schweda");
show_name("Josef W. Segur");
show_name("Jens Seidler");
show_name("Rytis Slatkevicius");
show_name("Peter Smithson");
show_name("Christian S&oslash;ttrup");
show_name("Michela Taufer");
show_name("Frank S. Thomas");
show_name("Mathias Walter");
show_name("Rom Walton");
show_name("Oliver Wang");
show_name("Frank Weiler");
show_name("Derek Wright");
echo "
</table>
<h2>Testers</h2>
<p>
BOINC alpha testers include:
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
Many thanks to Komori Hitoshi for repeatedly proof-reading this web site.
";
page_tail();
?>
