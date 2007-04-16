<?
require_once("docutil.php");
page_head("Project startup checklist");
echo "
<ul>
<li> Install auxiliary software (Apache, MySQL, PHP)
<li> Install BOINC software
<li> Develop an application
<li> Start the MySQL server
<li> Initialize the BOINC database
<li> Choose a master URL
<li> Put a page at the master URL pointing to the scheduling servers.
<li> Generate key pairs for code signing and upload authentication.
<li> Start the <b>feeder</b> program
</ul>
";
page_tail();
?>
