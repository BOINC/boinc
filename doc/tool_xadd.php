<?php
   require_once("docutil.php");
   page_head("xadd - tool for adding database items");

echo "
<b>xadd</b> adds platform and application records to the BOINC database.
Information is read from an XML file,
<b>project.xml</b>.
This file should be in the project's root directory.

<p>
The contents of <b>project.xml</b> should look like this:

<pre>", htmlspecialchars("
  <boinc>
    <app>
        <name>setiathome</name>
        <user_friendly_name>SETI@home</user_friendly_name>
    </app>
    ...
    <platform>
      <name>windows_intel</name>
      <user_friendly_name>Windows 95, 98, NT 2000, and XP</user_friendly_name>
    </platform>
    ...
  </boinc>
"), "</pre>

<p>
The 'name' of each item should be short
and without spaces or other special characters.
The 'user friendly name' (shown to end users) has no restrictions.
<p>
An example of projects.xml is in tools/ in the source tree.
<p>
This tool only adds new items;
entries that conflict with existing database entries are ignored.
";
page_tail();

?>

