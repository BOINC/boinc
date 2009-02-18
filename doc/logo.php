<?php
require_once("docutil.php");

page_head("Logos and graphics");

function art_list_head() {
    echo "
        <table cellpadding=8 border=1>
        <tr><th>Artist</th><th>Images (click for hi-res version)</th></tr>
    ";
}

function art_list_show($logos) {
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
}

echo "

<h2>The BOINC logo</h2>

The BOINC logo and associated icons were designed by Michal Krakowiak.
Its arms represent the convergence of separated things
(such as computers) into a unified whole.
The colors are based on U.C. Berkeley's blue-and-gold colors.
<ul>
<li> <a href=logo/www_logo.gif>164x73 version (GIFF)</a>
<li> <a href=logo/boinc_600.jpg>600x305 version (JPEG)</a>
<li> <a href=logo/boinc_logo_black.jpg>1280x535, black background (JPEG)</a>
<li> <a href=logo/boinc_logo.ai>Adobe Illustrator (.ai)</a>
<li> <a href=logo/boinc_logo.cdr>Vector graphics (.cdr)</a>
<li> <a href=logo/boinc_logo.pdf>Adobe PDF (.cdr)</a>
<li> <a href=logo/boinc_logo_ai.svg>Scalable Vector Graphics (.svg)</a>
<li> <a href=logo/boinc_logo.png>1280x535, transparent background (.png)</a>
<li> <a href=logo/boinc_logo.wmf>Windows meta-file (.wmf)</a>
<li> <a href=logo/logo_3d>3D versions (various sizes)</a>
</ul>

Icon:
<ul>
<li> Bitmaps (.bmp):
    <a href=logo/boinc16.bmp>16x16</a>
    <a href=logo/boinc32.bmp>32x32</a>
    <a href=logo/boinc48.bmp>48x48</a>
    <a href=logo/boinc128.bmp>128x128</a>
    <a href=logo/boinc256.bmp>256x256</a>
    <a href=logo/boinc_glossy2_512_F.tif>512x512</a> (.tif)
<li> <a href=logo/favicon.gif>Favicon (GIFF)</a>
</ul>
Installer splash screen:
<ul>
<li> <a href=logo/splash_8bit.png>Splash screen, 8-bit PNG</a>
<li> <a href=logo/splash_24bit.png>Splash screen, 24-bit PNG</a>
</ul>
'Powered by BOINC' image:
<p>
<a href=http://boinc.berkeley.edu><img src=logo/pb_boinc.gif></a>
<p>
(always link this to http://boinc.berkeley.edu)
<p>
Monochrome/black:
<p>
<img src=logo/boinc_watermark.png>

<p>

Wallpaper:
<p>
";
$wallpaper = array(
    array(
        "Robin Weezepoel",
        "<a href=images/BOINC_background_1_blue.jpg><img src=images/BOINC_background_1_blue_120.jpg></a>
        <a href=images/BOINC_background_1_grey.jpg><img src=images/BOINC_background_1_grey_120.jpg></a>
        <a href=images/BOINC_background_2_blue.jpg><img src=images/BOINC_background_2_blue_120.jpg></a>"
    ),
);
art_list_head();
art_list_show($wallpaper);
echo "
</table>
<h2>The old BOINC logo and related artwork</h2>
<ul>
<li>
The old logo in its native form as <a href=old_logo/logo.doc>a Word document</a>.
The logo uses the Planet Benson font from
<a href=http://www.larabiefonts.com>Larabie Fonts</a>.
<li>
Hi-res versions of the logo:
<a href=old_logo/logo.png>PNG</a>, <a href=old_logo/logo.jpg>JPEG</a>.
<li>
An icon for BOINC-related Podcasts, from Christian Beer:
<img align=top src=images/Logo_blau.jpg>
</ul>
<p>
The 'B in a circle' icon
<img src=old_logo/setup.PNG>was designed by Tim Lan.
The Mac variant was contributed by Juho Viitasalo.

<p>
Wallpaper:
<p>
<table cellpadding=8 border=1>
<tr><th>Artist</th><th>Images</th></tr>
<tr><td>Landi Paolo</td>
<td>
<a href=images/wallpaper.png><img src=images/wallpaper_120.png></a>
</td></tr>
</table>

<h2>Logo proposals</h2>
";
art_list_head();
echo "
";
$logos = array(
    array("Markus Beck",
        "<img width=400 src=logos/markus_beck.jpg>"
    ),
    array("Keijo Simonen",
        "<img src=logos/BOINC_logo_16b.png>
        <img src=logos/BOINC_logo_16c.png>"
    ),
    array("Markus Beck",
        "<img src=logos/markus/Logo3/BoincNeu2.jpg>
        <img src=logos/markus/Logo2/MarkusBoinc.jpg>
        <br>... and <a href=logos/markus>various others</a>"
    ),
    array("Invisible Design",
        "<img src=logos/logo.boinc.240x80-01.jpg>
        ... and <a href=logos/>many variants</a> in different sizes and colors,
        and for specific countries."
    ),
    array("Dario Arnaez",
        "<img src=logos/Boinc-Iso2.jpg>"
    ),
    array("Rebirther",
        "<img src=logos/rebirther/bluew.png>
        <img src=logos/rebirther/blackbackground.png>"
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
art_list_show($logos);

$banners = array(
    array(
        "Anthony Hern",
        array(
            array(
                "logos/Knightrider_TV_1.gif",
                "logos/Knightrider_TV_1.gif",
                90,
            ),
            array(
                "logos/Knightrider_TV_1n.gif",
                "logos/Knightrider_TV_1n.gif",
                90,
            ),
            array(
                "logos/Knightrider_TV_1tp.gif",
                "logos/Knightrider_TV_1tp.gif",
                90,
            ),
            array("<br>"),
            array(
                "images/hern_logo3.gif"
            ),
            array(
                "images/hern_logo4.gif"
            ),
        ),
    ),
    array("(unknown)",
        array(
            array("images/boincheading.jpg"),
            array("images/boincprojectheading.jpg"),
        ),
    ),
    array("Jared Hatfield",
        array(
            array(
                "images/hatfield.png",
                "images/hatfield_120.png",
            ),
        ),
    ),
    array("Myster65",
        array(
            array("logos/boinc-myster65-1.png"),
            array("logos/boinc-myster65-2.png"),
            array("<br>"),
            array("logos/boinc-myster65-3.png"),
            array("logos/boinc-myster65-4.png"),
        ),
    ),
);

echo "
    </table>
    <h2>Banners for BOINC projects</h2>
    <table cellpadding=8 border=1>
";

shuffle($banners);

foreach ($banners as $b) {
    $name = $b[0];
    echo "<tr><td>$name</td><td>";
    $imgs = $b[1];
    foreach ($imgs as $i) {
        $large = $i[0];
        $small = $i[1];
        $size = $i[2];
        if ($small) {
            if ($size) {
                echo "<a href=$large><img hspace=10 src=$small height=$size></a>";
            } else {
                echo "<a href=$large><img hspace=10 src=$small></a>";
            }
        } else {
            if ($large == "<br>") {
                echo $large;
            } else {
                echo  "<img hspace=10 src=$large>";
            }
        }
    }
    echo "</td></tr>
        ";
}

echo "
</table>

<p>
<table cellpadding=8 border=1>
</tr>

</td></tr>
</table>

<h2>Coinage</h2>
Tony/Knightrider/Chuggybus has created BOINC coinage.
See the <a href=images/coins/>large</a>
and <a href=images/coins_small>small</a> versions.
The Latin text means 'Berkeley open and shared resources'.


";

page_tail();
?>
