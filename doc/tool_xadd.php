<?php
   require_once("docutil.php");
   page_head("xadd - tool for adding database items");

echo "
<b>xadd</b> adds records to the BOINC database.
Information is read from an XML file,
<b>project.xml</b>.
This file should be in the project's root directory.

<p>
The contents of <b>project.xml</b> should look like this:

<pre>", htmlspecialchars("
  <boinc>
    <platform>
      <name>windows_intel</name>
      <user_friendly_name>Windows 95, 98, NT 2000, and XP</user_friendly_name>
    </platform>
    ...
  </boinc>
"), "</pre>

<p>
An example is in tools/ in the source tree.
<p>
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

