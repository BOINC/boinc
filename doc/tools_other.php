<?
require_once("docutil.php");
page_head("Operational tools: applications and versions");
echo "
<p>
BOINC provides a few tools for creating and operating projects:
<ul>
<li>
Utility programs (such as <b>add</b> and <b>create_work</b>).
These can be run manually or invoked from scripts.
<li>
C++ functions (such as <b>create_work()</b>).
<li>
Web interfaces (currently these provide only read access).
</ul>
Projects can create their own tools, either at a low level (e.g.
directly accessing the BOINC DB from PHP or Perl scripts) or by using
the BOINC DB C++ API (db/db.h).

<h3>The Add utility program</h3>
<p>
The program <b>add</b> performs various types of initialization:
<dl>
<dt>
add app -app_name name
</dt>
<dd>
Create a new application (just creates a DB record).
<dt>
add platform name -platform_name name
<dd>
Create a platform record (just creates a DB record);
<dt>
add app_version
<dd>
-app_name x
<dd>
-platform_name y
<dd>
-version a
<dd>
-download_dir d
<dd>
-download_url e
<dd>
-exec_dir b
<dd>
[ -message x ]
<dd>
[ -message_priority y ]
<dd>
[ -code_sign_keyfile x -exec_files file1 file2 ... ]
<dd>
[ -signed_exec_files file1 sign1 file2 sign2 ... ]
<dd>
<br>
Create an app_version record.
Copy the executable file(s) from
the compilation directory (-exec_dir) to the download directory.
If -exec_files is used, each executable file is signed using the given
private key; this should be used only for test/debug purposes.
If -signed_exec_files is used, the signatures are passed explicitly; this
should be used for production purposes, where the signatures are
generated on an offline computer.
If -message is used, the version is
tagged with the given message and optional priority.
<dt>
add user -email_addr x -name y -web_password z -authenticator a
<dd>
Create a user record.
<dt>
add prefs -email_addr x -prefs_file y
<dd>
Create a preference set, and make it the default preferences for
the given user.
</dl>

Common options:
<dl>
  <dd>
    -db_name name
  <dd>
    -db_passwd password
</dl>

<h3>Web Interfaces</h3>
<p>
The file <b>show_db.php</b> in the operational web site directory
displays the contents of the BOINC DB.
";
page_tail();
?>
