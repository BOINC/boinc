<?php

require_once("docutil.php");
require_once("download_network.inc");

page_head("Download BOINC add-on software");
echo "
    <p>
    You can download applications in several categories.
    <ul>
    <li>
    These applications are not endorsed by BOINC and
    you use them at your own risk.
    <li>
    We do not provide instructions for installing these applications.
    However, the author may have provided some help on installing or
    uninstalling the application.
    If this is not enough you should contact the author.
    Instructions for installing and running BOINC are
    <a href=participate.php>here</a>.
    <li>
    The <a href='http://www.boinc.dk/index.php?page=mirror_file_list'>BOINC Download Network</a>
    is designed and maintained by <a href='http://www.boinc.dk'>BOINC.dk</a> as a service for the BOINC community.
    Visit the BOINC Download Network site to add an entry to this list.

    </ul>
";
BOINC_download_network_print_download_links();
echo "
    <p><p>
";
page_tail();
?>
