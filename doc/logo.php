<?php
require_once("docutil.php");

page_head("Logos and graphics");

echo "

<h2>The current BOINC logo and icons</h2>
<ul>
<li>
The logo in its native form as <a href=logo/logo.doc>a Word document</a>.
The BOINC logo uses the Planet Benson font from
<a href=http://www.larabiefonts.com>Larabie Fonts</a>.
<li>
Hi-res versions of the logo:
<a href=logo/logo.png>PNG</a>, <a href=logo/logo.jpg>JPEG</a>.
<li>
An icon for BOINC-related Podcasts, from Christian Beer:
<img align=top src=images/Logo_blau.jpg>
</ul>
<p>
The 'B in a circle' icon
<img src=logo/setup.PNG>was designed by Tim Lan.
The Mac variant was contributed by Juho Viitasalo.

<h2>New BOINC logo?</h2>
<p>
We are looking for a new graphical identity for BOINC.
Our criteria:
<ul>
<li> It should work on light-colored backgrounds
(both the web) and also on black (for the screensaver).
<li> It should include graphics of various sizes,
including icons (16x16 up to 128x128),
a web-site logo (roughly 200x100),
and an installer splash screen (roughly 400x300).
These should all be recognizable and visually consistent.
<li>
It should appeal to people of all interests and demographics.
In particular, it should NOT have a futuristic,
high-tech, aggressive, or sci-fi look.
<li>
It would be good (though not vital) for the logo
to suggest global distributed computing,
or scientific research, or both.

</ul>
Here are some candidates.
If you have an opinion, or think you can do better,
   please <a href=contact.php>contact us</a>.
<p>
<table cellpadding=8 border=1>
<tr><th>Artist</th><th>Images (click for hi-res version)</th></tr>
";
$logos = array(
    array("Markus Beck",
        "<img src=logos/markus/Logo3/BoincNeu2.jpg>
        <img src=logos/markus/Logo2/MarkusBoinc.jpg>
        <br>... and <a href=logos/markus>various others</a>"
    ),
    array("<a href=http://www.boincproject.org/>Invisible Design</a>",
        "<img src=logos/logo.boinc.240x80-01.jpg>
        ... and <a href=logos/>many variants</a> in different sizes and colors,
        and for specific countries."
    ),
    array("Dario Arnaez",
        "<img src=logos/Boinc-Iso2.jpg>"
    ),
    array("Rebirther",
        "<img src=logo/rebirther/bluew.png>
        <img src=logo/rebirther/blackbackground.png>"
    ),
    array("Michal Krakowiak",
        "<img src=logos/boinc_krakowiak.jpg>
        <br>
        16x16 icon: <img src=logos/boinc_small_icon_krakowiak.jpg>"
    ),
    array("Jim Paulis",
        "<a href=images/BOINC_logo.jpg><img src=images/BOINC_logo_240.jpg></a>
        ... and several variants"
    ),
    array("Eduardo Ascarrunz",
        "<a href=images/BOINC_01.png><img height=90 src=images/BOINC_01.png></a>
        <a href=images/BOINC_02.png><img height=90 src=images/BOINC_02.png></a>
        <a href=images/BOINC_03.png><img height=90 src=images/BOINC_03.png></a>
        <a href=images/BOINC_04.png><img height=90 src=images/BOINC_04.png></a>
        <br>
        <a href=images/BOINC.ai>Adobe Illustrator source</a>"
    ),
    array("Michael Peele",
        "<a href=images/NewBOiNC.gif><img src=images/NewBOiNC_120.png></a>,
        <a href=images/BOiNC3.jpg><img src=images/BOiNC3_120.png></a>
        <a href=images/BOiNC2.png><img src=images/BOiNC2_120.png></a>"
    ),
    array("Yopi at sympatico.ca",
        "<a href=images/boincb6.png><img src=images/boincb6_120.png>
        <a href=images/boincb7.png><img src=images/boincb7_120.png>"
    ),
);
shuffle($logos);
foreach($logos as $logo) {
    $x0 = $logo[0];
    $x1 = $logo[1];
    echo "<tr>
        <td>$x0</td>
        <td>$x1</td>
        </tr>
    ";
}
echo "
</table>

<h2>Banners for BOINC projects</h2>
<p>
<table cellpadding=8 border=1>
<tr><th>Artist</th><th>Images</th></tr>
<tr>
<td>Tony Hern</td>
<td>
<a href=logos/Knightrider_TV_1.gif><img height=90 src=logos/Knightrider_TV_1.gif></a>
<a href=logos/Knightrider_TV_1n.gif><img height=90 src=logos/Knightrider_TV_1n.gif></a>
<a href=logos/Knightrider_TV_1tp.gif><img height=90 src=logos/Knightrider_TV_1tp.gif></a>
</td>
</tr>
<tr>
<td>Anthony Hern</td>
<td>
<img src=images/hern_logo3.gif>
<br>
<img src=images/hern_logo4.gif>
</td></tr>

<td>(unknown)</td>
<td>
<img src=images/boincheading.jpg> 
<img src=images/boincprojectheading.jpg>
</td></tr>
<td>Jared Hatfield</td>
<td>
<a href=images/hatfield.png><img src=images/hatfield_120.png></a>
</td></tr>
</table>

<h2>Coinage</h2>
Tony/Knightrider/Chuggybus has created BOINC coinage.
See the <a href=images/coins/>large</a>
and <a href=images/coins_small>small</a> versions.
The Latin text means 'Berkeley open and shared resources'.

<h2>Wallpaper</h2>
<table cellpadding=8 border=1>
<tr><th>Artist</th><th>Images</th></tr>
<tr><td>Landi Paolo</td>
<td>
<a href=images/wallpaper.png><img src=images/wallpaper_120.png></a>
</td></tr>
</table>

";

page_tail();
?>
