<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/cache.inc");


start_cache(3600);
page_head("Download BOINC add-on software");
echo "
    <p>
    You can download applications in several categories.
    <ul>
    <li>
    These applications are not endorsed by ".PROJECT." and
    you use them at your own risk.
    <li>
    We do not provide instructions for installing these applications.
    However, the author may have provided some help on installing or
    uninstalling the application.
    If this is not enough you should contact the author.
    Instructions for installing and running BOINC are
    <a href=http://boinc.berkeley.edu/participate.php>here</a>.
    <li>
    This list is managed centrally at <a href='http://boinc.berkeley.edu'>the BOINC website</a>.
    </ul>
";

$httpFile = @fopen("http://boinc.berkeley.edu/addons.php?strip_header=true", "rb");
if (!$httpFile){
    echo "";
} else {
    fpassthru($httpFile);
    fclose($httpFile);
}

echo "
    <p><p>
";
page_tail();
end_cache();
?>
