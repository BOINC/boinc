<?php

require_once("docutil.php");
page_head("xadd - tool for adding database items");

echo "
<b>xadd</b> adds platform and application records to the BOINC database.
Information is read from an XML file <b>project.xml</b>
in the project's root directory.
Run <code>xadd</code> from the project root directory, i.e.:
<pre>
cd ~/projects/project_name
bin/xadd
</pre>

<p>
The contents of <b>project.xml</b> should look like this:

<pre>", htmlspecialchars("
  <boinc>
    <app>
        <name>setiathome</name>
        <user_friendly_name>SETI@home</user_friendly_name>
        [ <beta>1</beta> ]
    </app>
    ...
    <platform>
        <name>anonymous</name>
        <user_friendly_name>anonymous</user_friendly_name>
    </platform>
    <platform>
        <name>i686-pc-linux-gnu</name>
        <user_friendly_name>Linux/x86</user_friendly_name>
    </platform>
    <platform>
        <name>windows_intelx86</name>
        <user_friendly_name>Windows/x86</user_friendly_name>
    </platform>
    <platform>
        <name>powerpc-apple-darwin</name>
        <user_friendly_name>Mac OS X</user_friendly_name>
    </platform>
    ...
  </boinc>
"), "</pre>

<p>
The 'name' of each item should be short
and without spaces or other special characters.
The 'user friendly name' (shown to end users) has no restrictions.
An example project.xml file is in source/tools/.
<p>
<code>xadd</code> only adds new items;
entries that conflict with existing database entries are ignored.
";
page_tail();

?>

