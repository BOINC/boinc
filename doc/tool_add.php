<? // -*- html -*-
   // $Id$
   require_once("docutil.php");
   page_head("<code>add</code> Tool");
   ?>

`<code>add</code>' adds objects to the BOINC database through the command-line.


Usages:
<pre>

add project      --name=yah --long_name="YETI @ home"

add platform     --name=c64 [ --user_friendly_name="Commodore 64" ]

add core_version --platform=c64 --version_num=717
                    --exec_file=/path/to/boinc_7.17_c64
                    [--message="Message"] [--message_priority="Priority"]

add app          --name=YetiApp [--min_version=716]

add app_version  --app=YetiApp --platform=c64 --version_num=717
                    --exec_file=/path/to/yeti_7.17_c64
                      [--signature_file=/path/to/sig_file]
                    [--exec_file=/path/to/more_bins
                      [--signature_file=/tmp/sig_file2]] ...

add user         --name="Carl Sagan" --email_addr="carl.sagan@example.com"
                    --authenticator="deadbeef"
                    [--country=Estonia --postal_code=94703
                     --global_prefs_file=/path/to/prefs.xml]

</pre>

For detailed help, type `<code>add</code>' (with no arguments) or `<code>add
app_version</code>' for help on adding an app_version, etc.

<p>
Typically you will only <code>add</code> <code>platform</code>s
and <code>app</code>s, and only at the inception of the project.
A project is created with <a href=make_project.php>make_project</a>.
Core_version and app_version are automatically added by the
<a href=tool_update_versions.php>update_versions</a> tool.
Users are usually created through the Web interface.

<?
   page_tail();
?>

