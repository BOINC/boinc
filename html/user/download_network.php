<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/download_network.inc");

init_session();

page_head("Download BOINC add-on software");
echo "
    You can download applications from any of the following categories.
    Please note that these applications are not endorsed by ".PROJECT." and that you use them at your own risk:<p>\n
";
BOINC_download_network_print_download_links();
echo "
    <p>
    Instructions for installing and running BOINC are
    <a href=http://boinc.berkeley.edu/client.php>here</a>.
    <p>
    We do not provide instructions for installing the above applications.
    However, the author may have provided some help on installing or
    uninstalling the application.
    If this is not enough you should contact the author.
    <p>
    <font size=-1>
    The <a href='http://www.boinc.dk/index.php?page=mirror_file_list'>BOINC Download Network</a>
    is designed and maintained by <a href='http://www.boinc.dk'>BOINC.dk</a> as a service for the BOINC community.
    </font>
";
page_tail();
?>
