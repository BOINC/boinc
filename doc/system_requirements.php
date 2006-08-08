<?php

require_once("docutil.php");
require_once("../html/inc/translation.inc");

page_head(tr(SRQ_PAGE_TITLE));

echo "
<b>".tr(SRQ_INTRO)."</b>

<hr>

<h2>".tr(SRQ_MSWIN)."<h2>
<h4>".tr(SRQ_OS)."</h4>
<ul>
<li> ".tr(SRQ_MSWIN_REQ_OS)."
</ul>
<h4>".tr(SRQ_MIN_HARDWARE)."</h4>
<ul>
<li> ".tr(SRQ_MSWIN_REQ_CPU)."
<li> ".tr(SRQ_MSWIN_REQ_RAM)."
<li> ".tr(SRQ_MSWIN_REQ_DISK)."
 </ul>


<hr>
<h2>".tr(SRQ_APPLMAC)."</h2>
<h4>".tr(SRQ_OS)."</h4>
<ul>
<li> ".tr(SRQ_APPLMAC_REQ_OS)."
</ul>
<h4>".tr(SRQ_MIN_HARDWARE)."</h4>
<ul>
<li> ".tr(SRQ_APPLMAC_REQ_CPU)."
<li> ".tr(SRQ_APPLMAC_REQ_RAM)."
<li> ".tr(SRQ_APPLMAC_REQ_DISK)."
</ul>

<hr>
<h2>".tr(SRQ_LINUX)."</h2>
<h4>".tr(SRQ_OS)."</h4>
<ul>
<li> ".tr(SRQ_LINUX_REQ_KERNEL)."
<li> ".tr(SRQ_LINUX_REQ_GLIBC)."
<li> ".tr(SRQ_LINUX_REQ_XFREE86)."
<li> ".tr(SRQ_LINUX_REQ_GTKPLUS)."
</ul>
<h4>".tr(SRQ_MIN_HARDWARE)."</h4>
<ul>
<li> ".tr(SRQ_LINUX_REQ_CPU)."
<li> ".tr(SRQ_LINUX_REQ_RAM)."
<li> ".tr(SRQ_LINUX_REQ_DISK)."
</ul>
<hr>
";

page_tail(true);

?>

