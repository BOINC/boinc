<?php
require_once("docutil.php");
page_head("Sending files");
echo "
To send a file to a specific host, use the function
<pre>
send_file(int host_id, const char* file_name, int priority, long int exp_date) 
</pre>
or the command line program
<pre>
send_file -host_id X -file_name Y -priority Z -days_exp N
</pre>
<ul>
<li> priority is the relative importance of the file (default = 1)
<li> days_exp is the number of days the file should stay on the 
host (default = 100)
</ul>
<p>
send_file creates a result and initializes it with a name of the form
send_FILENAME_HOSTID_TIME.
<p>
A message is created for the host and added to the msg_to_host table. 
This message instructs the client to download the file
and acknowledge a successful download.
The download message included in the next RPC reply to the host.
The message has the form:
", html_text("
<app>
	<name>FILE_MOVER</name>
</app>
<app_version>
	<app_name>FILE_MOVER</app_name>
	<version_num>n</version_num>
</app_version>
<result>
    <wu_name>x</wu_name>
    <name>y</name>
</result>
<file_info>
	<name>file_name</name>
	<url>download_dir/file_name</url>
	<md5_cksum>md5</md5_cksum>
	<nbytes>file->nbytes</nbytes>
	<sticky/>
	<priority>priority</priority>
	<exp_date>exp_date</exp_date>
</file_info>
<workunit>
	<name>result.name</name>
	<app_name>FILE_MOVER</app_name>
	<file_ref>
		<file_name>file_name</file_name>
	</file_ref>
</workunit>"), "
";
page_tail();
?>
