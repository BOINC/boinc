<? // -*- html -*-
   // $Id$
   require_once("docutil.php");
   page_head("<code>xadd</code> Tool");
   ?>

`<code>xadd</code>' adds objects to the BOINC database through an XML
file, <code><b>project.xml</b></code>.  This file should be in the same
location as <code>config.xml</code>.

The contents of <code>project.xml</code> should look like this:

<pre>
  &lt;boinc&gt;
    &lt;project&gt;
      &lt;short_name&gt;yah&lt;/short_name&gt;
      &lt;long_name&gt;YETI @ Home&lt;/long_name&gt;
    &lt;/project&gt;
    &lt;platform&gt;
      &lt;name&gt;c64&lt;/name&gt;
      &lt;user_friendly_name&gt;Commodore 64&lt;/user_friendly_name&gt;
    &lt;/platform&gt;
    ...
  &lt;/boinc&gt;
</pre>

Notes:
<ul>
  <li>Object arguments have the same format as for the `add' tool
    command-line arguments.  See also the <a href=tool_add.php>documentation
    for `add'</a>.
  <li>This tool (currently) only adds new items; thus:
    <ul>
      <li>Entries that conflict with existing database entries are ignored;
        this includes entries that have changed.
    </ul>
</ul>

<?
   page_tail();
?>

