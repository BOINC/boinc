<?php
   require_once("docutil.php");
   page_head("xadd - tool for adding database items");

echo "
`<code>xadd</code>' adds records to the BOINC database through an XML file,
<code><b>project.xml</b></code>.
This file should be in the same location as <code>config.xml</code>.

<p>
The contents of <code>project.xml</code> should look like this:

<pre>", htmlspecialchars("
  <boinc>
    <project>
      <short_name>setiathome</short_name>
      <long_name>SETI@home</long_name>
    </project>
    <platform>
      <name>windows_intel</name>
      <user_friendly_name>Windows 95, 98, NT 2000, and XP</user_friendly_name>
    </platform>
    ...
  </boinc>
"), "</pre>

Notes:
<ul>
  <li>Object arguments have the same format as for the `add' tool
    command-line arguments.  See also the <a href=tool_add.php>documentation
    for `add'</a>.
  <li>This tool (currently) only adds new items;
      Entries that conflict with existing database entries are ignored.
</ul>
";
   page_tail();
?>

