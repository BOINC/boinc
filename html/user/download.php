<?php

require_once("../inc/cache.inc");
require_once("../inc/util.inc");

start_cache(DOWNLOAD_PAGE_TTL);

require_once("../inc/db.inc");
require_once("../inc/download.inc");


db_init();

page_head("Download BOINC software");
echo "
    <font color=ff0000>
    <b>First-time ".PROJECT." participants</b>:
    <br>Don't download BOINC software now.
    <a href=create_account_form.php>Create an account</a> first.
    </font>
    <p>
";
print_download_links();
echo "
    <p>
    Instructions for installing and running BOINC are
    <a href=http://boinc.berkeley.edu/participate.php>here</a>.
    <p>
    If your computer is not one of the above types,
    you can
    <ul>
    <li> <a href=http://boinc.berkeley.edu/anonymous_platform.php>download and compile the BOINC software yourself</a> or
    <li> <a href=download_other.php>download from a third-party site</a>.
    </ul>
    <p>
    BOINC can be customized for
    <a href=http://boinc.berkeley.edu/language.php>languages other than English</a>
    <p>
    <font size=-1>
    <a href=http://boinc.berkeley.edu>BOINC</a>
    is distributed computing software
    developed at the University of California by
    the SETI@home project.
    </font>
";
page_tail();
end_cache(DOWNLOAD_PAGE_TTL);
?>
