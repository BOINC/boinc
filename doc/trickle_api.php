<?php
require_once("docutil.php");
page_head("Trickle message API");

echo "
The interface for <a href=trickle.php>trickle messages</a>
includes both client-side and server-side components.
<h3>Client-side API</h3>
To send a trickle-up message, call
<pre>
<code>int boinc_send_trickle_up(char* variety, char* text)</code>
</pre>
<p>
To receive a trickle-down message, call
<pre>
<code>int boinc_receive_trickle_down(char* buf, int len)</code>
</pre>
This returns true (nonzero) if there was a message.

<h3>Server-side API</h3>

<p>
To handle trickle-up messages, use a 'trickle_handler' daemon.
This is a program, based on sched/trickle_handler.C, linked with a function
",html_text("
int handle_trickle(MSG_FROM_HOST&);

struct MSG_FROM_HOST {
    int create_time;
    int hostid;
    char variety[256];              // project-defined; what kind of msg
    char xml[MSG_FROM_HOST_BLOB_SIZE];
};

"),"

This function should return zero if the message was handled successfully;
otherwise it will be retried later.
The 'hostid' field identifies the host from which the message was sent.
The daemon must be passed a '-variety X' command-line argument,
telling it what kind of messages to handle.
The daemon should be specified in the
<a href=project_daemons.php>project configuration file</a>.
<p>
To send send trickle-down messages
(from a trickle handler daemon or other program)
you must insert a record in the 'msg_to_host' table.
From C/C++, this is done as follows:
",html_text("
DB_MSG_TO_HOST mth;

mth.clear();
mth.create_time = time(0);
mth.hostid = hostid;
sprintf(mth.xml,
    \"<trickle_down>\\n\"
    \"   <result_name>%s</result_name>\\n\"
    \"   ...\\n\"
    \"</trickle_down>\\n\",
    ...
);
retval = mth.insert();
"),"

<p>
To send trickle-down messages, a project must include the line
<pre>
&lt;msg_to_host/>
</pre>
in the <a href=configuration.php>configuration</a> (config.xml) file.


";
page_tail();
?>
