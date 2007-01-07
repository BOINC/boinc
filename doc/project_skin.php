<?php
require_once("docutil.php");
page_head("Project graphics in the BOINC simple GUI");
echo "
<p>
The 'simple GUI' available in versions 5.8+ of the BOINC Manager
uses graphical representations of projects and applications:
<ul>
<li> The project is represented by a 40x40 pixel icon.
<li> Each application is represented by a sequence of images,
each up to 290x126 pixels.
These are shown as a slideshow, changing once every few seconds.
</ul>
For example, in the following screenshot of the simple GUI,
the two icons at the bottom represent CPDN and SETI@home,
and the earth-map image in the middle represents
the particular CPDN application that is currently running.
<center>
<img vspace=10 src=images/newboinc.jpg>
</center>

<h2>Specifying project files</h2>
<p>
Project graphics files are specified in a configuration
file <b>project_files.xml</b>
that you put in your project's root directory.
This file specifies a set of 'project files' that will
be automatically downloaded to clients
(this can be used for purposes other than graphics).
<p>
The format of 'project_files.xml' is:
".html_text("
<file_info>
    <name>X</name>
    <url>X</url>
    <md5_cksum>X</md5_cksum>
</file_info>
...
<project_files>
    <file_ref>
        <file_name>X</file_name>
        <open_name>X</open_name>
    </file_ref>
    ...
</project_files>
")."
<p>
For each file, this specifies:
<ul>
<li> its URL (where to download it from)
<li> its physical name; it will be stored in the project
directory on the client under this name.
<li> its MD5 checksum (use 'openssl dgst' to get this).
<li> its logical name; a 'soft link' file will be created
with this name, linking to the physical name.
</ul>
All file_info records must appear before the project_files record.
<p>
As with all BOINC files, project files are immutable.
If you want to change the contents of a file,
you must use a new physical name.
<p>


<h4>Project Icon</h4>
<p>
A project icon is a 40x40 image,
PNG, JPG, GIF, or BMP format.
<p>
An example would look like this:
".html_text("
<file_info>
    <name>stat_icon_01.png</name>
    <url>http://www.example.com/download/stat_icon_01.png</url>
    <md5_cksum>186c5385c8f2a48ccc7e4f64251fcda1</md5_cksum>
</file_info>
<project_files>
    <file_ref>
      <file_name>stat_icon_01.png</file_name>
      <open_name>stat_icon</open_name>
    </file_ref>
</project_files>
")."
<p>
Two things to note here:
<ul>
  <li>The '01' in stat_icon_01.png is used for versioning.
  <li>The physical name for the project icon is 'stat_icon_01.png'
  while the logical name for the project icon is 'stat_icon'.
  The manager looks for 'stat_icon' and resolves
  it to a physical name. 
</ul>

<h4>Application Slideshow</h4>
<p>
You can have one or more images displayed in the Simple GUI
when BOINC is running one of your apps.
Each image can have a height up to 126px and a width up to 290px,
and can be any of the following image types: PNG, JPG, GIF, and BMP.
<p>
An example would look like this:
".html_text("
<file_info>
    <name>slideshow_exampleapp_01_01.png</name>
    <url>http://www.example.com/download/slideshow_exampleapp_01_01.png</url>
    <md5_cksum>186c5385c8f2a48ccc7e4f64251fcda1</md5_cksum>
</file_info>
<file_info>
    <name>slideshow_exampleapp_02_01.png</name>
    <url>http://www.example.com/download/slideshow_exampleapp_02_01.png</url>
    <md5_cksum>3b262da3d69d6b9eb55add88b66cdab4</md5_cksum>
</file_info>
</file_info>
<project_files>
    <file_ref>
      <file_name>slideshow_exampleapp_01_01.png</file_name>
      <open_name>slideshow_exampleapp_01</open_name>
    </file_ref>
    <file_ref>
      <file_name>slideshow_exampleapp_02_01.png</file_name>
      <open_name>slideshow_exampleapp_02</open_name>
    </file_ref>
</project_files>
")."
<p>
In this example:<br>
<pre>
slideshow_exampleapp_02_01.png
</pre>
<p>
'slideshow_' labels it as a slideshow file, 'exampleapp' is the 
application short name,
'02' is the index of the slide within the slideshow,
and 01 is the version of the file.
<p>

<h3>Example</h3>
<p>
Here is the 'project_files.xml' file SETI@home is using:
".html_text("
<file_info>
    <name>arecibo_181.png</name>
    <url>http://setiathome.berkeley.edu/sg_images/arecibo_181.png</url>
    <md5_cksum>f9b65230a594098d183d2266511bc648</md5_cksum>
</file_info>
<file_info>
    <name>sah_40.png</name>
    <url>http://setiathome.berkeley.edu/sg_images/sah_40.png</url>
    <md5_cksum>5791ba1be2d33eaa5f90ecf5de89a53d</md5_cksum>
</file_info>
<file_info>
    <name>sah_banner_290.png</name>
    <url>http://setiathome.berkeley.edu/sg_images/sah_banner_290.png</url>
    <md5_cksum>39839286db7f580bef5377322d15ed35</md5_cksum>
</file_info>
<file_info>
    <name>sah_ss_290.png</name>
    <url>http://setiathome.berkeley.edu/sg_images/sah_ss_290.png</url>
    <md5_cksum>caf95504208aedd6ac6d82201e2fd8b1</md5_cksum>
</file_info>
<project_files>
    <file_ref>
      <file_name>sah_40.png</file_name>
      <open_name>stat_icon</open_name>
    </file_ref>
    <file_ref>
      <file_name>sah_ss_290.png</file_name>
      <open_name>slideshow_setiathome_enhanced_00</open_name>
    </file_ref>
    <file_ref>
      <file_name>arecibo_181.png</file_name>
      <open_name>slideshow_setiathome_enhanced_01</open_name>
    </file_ref>
    <file_ref>
      <file_name>sah_banner_290.png</file_name>
      <open_name>slideshow_setiathome_enhanced_02</open_name>
    </file_ref>
</project_files>
")."
";
page_tail();
?>
