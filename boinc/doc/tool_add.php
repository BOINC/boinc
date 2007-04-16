<?
   require_once("docutil.php");
   page_head("add - tool for adding database items");

echo "
<b>add</b> is a command-line tool for adding entries to the BOINC database.

<p>
<b>add is deprecated - we recommend using <a href=tool_xadd.php>xadd</a> instead.</b>
<p>

Usages:
<pre>

add project      --name=sah --long_name=\"SETI@home\"

add platform     --name=windows [ --user_friendly_name=\"Windows (95, NT, XP)\" ]

add core_version --platform=c64 --version_num=717
                    --exec_file=/path/to/boinc_7.17_c64
                    [--message=\"Message\"] [--message_priority=\"Priority\"]

add app          --name=YetiApp [--min_version=716]

add app_version  --app=YetiApp --platform=c64 --version_num=717
                    --exec_file=/path/to/yeti_7.17_c64
                      [--signature_file=/path/to/sig_file]
                    [--exec_file=/path/to/more_bins
                      [--signature_file=/tmp/sig_file2]] ...

add user         --name=\"Carl Sagan\" --email_addr=\"carl.sagan@example.com\"
                    --authenticator=\"deadbeef\"
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
";
   page_tail();
?>

