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
  &lt;boinc&rt;
    &lt;project&rt;
      &lt;short_name&rt;yah&lt;/short_name&rt;
      &lt;long_name&rt;YETI @ Home&lt;/long_name&rt;
    &lt;/project&rt;
    &lt;platform&rt;
      &lt;name&rt;c64&lt;/name&rt;
      &lt;user_friendly_name&rt;Commodore 64&lt;/user_friendly_name&rt;
    &lt;/platform&rt;
    ...
  &lt;/boinc&rt;
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

