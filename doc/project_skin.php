<?php
require_once("docutil.php");
page_head("Customizing the Project for the BOINC Manager skin");
echo "
<h3>Contents</h3>
<ul>
    <li><a href=\"#CustomizeSimpleGUI\">Custom Project Elements within the Simple GUI</a>
        <ul>
            <li><a href=\"#Icon\">Project Icon</a>
            <li><a href=\"#Slideshow\">Project Application Slideshow</a>
            <li><a href=\"#Sample\">Sample</a>
        </ul>
</ul>

<h3><a name=\"CustomizeSimpleGUI\">Custom Project Elements within the Simple GUI</a></h3>
<p>
A 'project_files.xml' file can be created for the purposes of sending project customization
elements to the BOINC client's Simple GUI. The file should be created in the project root
directory.
<p>
The contents of 'project_files.xml' should look like this:
"; block_start(); echo "
&lt;file_info /&gt;
&lt;file_info /&gt;
...
&lt;project_files&gt;
    &lt;file_ref /&gt;
    &lt;file_ref /&gt;
    ...
&lt;/project_files&gt;
"; block_end(); echo "
<p>
It is important to note that all file_info records must appear before the project_files
record.
<p>


<h4><a name=\"Icon\">Project Icon</a></h4>
<p>
A project icon is a 40x40 image that will uniquely identify your project within the simple
GUI. Project icons can be any of the following image types: PNG, JPG, GIF, and BMP.
<p>
An example would look like this:
"; block_start(); echo "
&lt;file_info&gt;
    &lt;name>stat_icon_01.png&lt;/name&gt;
    &lt;url>http://www.example.com/download/stat_icon_01.png.gz&lt;/url&gt;
&lt;/file_info&gt;
&lt;project_files&gt;
    &lt;file_ref&gt;
      &lt;file_name>stat_icon_01.png&lt;/file_name&gt;
      &lt;open_name>stat_icon&lt;/open_name&gt;
    &lt;/file_ref&gt;
&lt;/project_files&gt;
"; block_end(); echo "
<p>
Two things to note here:
<ul>
  <li>The '01' in stat_icon_01.png is used for versioning, in case you want to eventually
      change your icon without causing the client to resort to using the default icon for
      a short period of time.
  <li>The physical name for the project icon is 'stat_icon_01.png' while the logical name 
      for the project icon is 'stat_icon'. The manager is looking for 'stat_icon'. 
</ul>

<h4><a name=\"Slideshow\">Project Application Slideshow</a></h4>
<p>
You can have one or more images displayed in the Simple GUI when BOINC is processing your
task. Each image can have a height of 126px and a width of 290px, and can be any of the 
following image types: PNG, JPG, GIF, and BMP.
<p>
An example would look like this:
"; block_start(); echo "
&lt;file_info&gt;
    &lt;name>slideshow_exampleapp_01_01.png&lt;/name&gt;
    &lt;url>http://www.example.com/download/slideshow_exampleapp_01_01.png&lt;/url&gt;
&lt;/file_info&gt;
&lt;file_info&gt;
    &lt;name>slideshow_exampleapp_02_01.png&lt;/name&gt;
    &lt;url>http://www.example.com/download/slideshow_exampleapp_02_01.png&lt;/url&gt;
&lt;/file_info&gt;
&lt;project_files&gt;
    &lt;file_ref&gt;
      &lt;file_name>slideshow_exampleapp_01_01.png&lt;/file_name&gt;
      &lt;open_name>slideshow_exampleapp_01&lt;/open_name&gt;
    &lt;/file_ref&gt;
    &lt;file_ref&gt;
      &lt;file_name>slideshow_exampleapp_02_01.png&lt;/file_name&gt;
      &lt;open_name>slideshow_exampleapp_02&lt;/open_name&gt;
    &lt;/file_ref&gt;
&lt;/project_files&gt;
"; block_end(); echo "
<p>
In this example:<br>
slideshow_exampleapp_02_01.png
<p>
'slideshow_' is the beginning of what the manager is looking for, 'exampleapp' is the 
applications short name for which the slideshow belongs too, '02' is the index of the 
slide within the slideshow, and 01 is the version of the file.
<p>

<h3><a name=\"Sample\">Sample</a></h3>
<p>
Here is the 'project_files.xml' file SETI@Home is using:
"; block_start(); echo "
&lt;file_info&gt;
    &lt;name&gt;arecibo_181.png&lt;/name&gt;
    &lt;url&gt;http://setiathome.berkeley.edu/sg_images/arecibo_181.png&lt;/url&gt;
&lt;/file_info&gt;
&lt;file_info&gt;
    &lt;name&gt;sah_40.png&lt;/name&gt;
    &lt;url&gt;http://setiathome.berkeley.edu/sg_images/sah_40.png&lt;/url&gt;
&lt;/file_info&gt;
&lt;file_info&gt;
    &lt;name&gt;sah_banner_290.png&lt;/name&gt;
    &lt;url&gt;http://setiathome.berkeley.edu/sg_images/sah_banner_290.png&lt;/url&gt;
&lt;/file_info&gt;
&lt;file_info&gt;
    &lt;name&gt;sah_ss_290.png&lt;/name&gt;
    &lt;url&gt;http://setiathome.berkeley.edu/sg_images/sah_ss_290.png&lt;/url&gt;
&lt;/file_info&gt;
&lt;project_files&gt;
    &lt;file_ref&gt;
      &lt;file_name&gt;sah_40.png&lt;/file_name&gt;
      &lt;open_name&gt;stat_icon&lt;/open_name&gt;
    &lt;/file_ref&gt;
    &lt;file_ref&gt;
      &lt;file_name&gt;sah_ss_290.png&lt;/file_name&gt;
      &lt;open_name&gt;slideshow_setiathome_enhanced_00&lt;/open_name&gt;
    &lt;/file_ref&gt;
    &lt;file_ref&gt;
      &lt;file_name&gt;arecibo_181.png&lt;/file_name&gt;
      &lt;open_name&gt;slideshow_setiathome_enhanced_01&lt;/open_name&gt;
    &lt;/file_ref&gt;
    &lt;file_ref&gt;
      &lt;file_name&gt;sah_banner_290.png&lt;/file_name&gt;
      &lt;open_name&gt;slideshow_setiathome_enhanced_02&lt;/open_name&gt;
    &lt;/file_ref&gt;
&lt;/project_files&gt;
"; block_end(); echo "
";
page_tail();
?>
