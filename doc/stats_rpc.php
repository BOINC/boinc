<?php

require_once("docutil.php");

page_head("Web services for statistics sites");

echo "
The following 'web service' is available for statistics sites.
";
list_start();
list_item("URL", "WEBSITE_URL/verify_cpid.php?authenticator=x&cpid=y");
list_item("inputs", "x=authenticator (also known as 'account key');
    <br> y=user cross-project ID, as listed in XML stats file
    or user account page"
);
list_item("output",
    "XML: enclosing &lt;reply> tag, and either
    <ul>
    <li> &lt;success>&lt;/success>
    if authenticator is correct for that user
    (viewed in a browser, this may be displayed as &lt;success/>).
    <li> &lt;error>error_message&lt;/error>
    otherwise.
    </ul>
    "
);
list_end();

page_tail();
?>
