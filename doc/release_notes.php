<?php
require_once("docutil.php");
require_once("../html/inc/translation.inc");

page_head(tr(RLN_PAGE_TITLE));

echo "
<ul>
    <li> <a href=#new>".sprintf(tr(RLN_WHATS_NEW),"5.4")."</a>
    <li> <a href=#install>".tr(RLN_INSTALLING)."</a>
    <li> <a href=#uninstall>".tr(RLN_UNINSTALLING)."</a>
    <li> <a href=#issues>".tr(RLN_KNOWN_ISSUES)."</a>
    <li> <a href=#troubleshoot>".tr(RLN_TROUBLESHOOT)."</a>
</ul>

<h2>".sprintf(tr(RLN_WHATS_NEW),"5.4")."</h2>
<ul>
<li> ".tr(RLN_NEWF_5_4_AM)."
<li> ".sprintf(tr(RLN_NEWF_5_4_PREF_OVERRIDE),
          "<a href=http://boinc.berkeley.edu/prefs_override.php>","</a>")."
<li> ".tr(RLN_NEWF_5_4_ALERTS_CONNECT)."
</ul>

".sprintf(tr(RLN_RECOMMEND_LATEST_VERSION),"5.4")."

<p>
".sprintf(tr(RLN_LINK2_VERSION_HISTORY),"<a href=rev_history.php>","</a>")."

<a name=install></a>
<h2>".tr(RLN_INSTALLING)."</h2>

<h3>".tr(RLN_MSWIN)."</h3>
".tr(RLN_MSWIN_INSTALL_MODES)."

<ul>
<li>
<b>".tr(RLN_SINGLE_USER_INSTALL)."</b>
<p>
".tr(RLN_MSWIN_INSTALL_SINGLE_USER_DESC)."

<li>
<b>".tr(RLN_SHARED_INSTALL)."</b>
<p>
".tr(RLN_MSWIN_INSTALL_SHARED_DESC)."

<li>
<b>".tr(RLN_WIN_SERVICE_INSTALL)."</b>
<p>
".tr(RLN_MSWIN_INSTALL_WINSERVICE_DESC)."
</ul>
".sprintf(tr(RLN_WIN_INSTALL_PROBLEMS), "<ul><li>", "<b>", "</b>", "<li>", "<a href=http://boinc.berkeley.edu/download_all.php>", "</a>", "<li>", "<a href=http://support.microsoft.com/kb/290301>", "</a>", "</ul>")."
<h3>Mac OS X</h3>
<ul>
<li> If your browser has not already done so,
expand the zip archive by double-clicking on it in the Finder.
<li> Double-click on <b>BOINC.pkg</b> to run the installer,
then follow the prompts.
<li> Close the installer when it is finished.
This will automatically launch the BOINC Manager.
<li> If you want BOINC to be your screen saver,
open <b>System Preferences</b> from the Apple menu.
Select <b>Desktop & Screen Saver</b> and select <b>BOINCSaver</b>.
</ul>
<p>
Several <a href=mac_admin_tools.php>tools</a> for Macintosh system administrators are available to:
<ul>
<li> automatically run BOINC as a daemon or system service at boot time
<li> improve security for stand-alone clients 
<li> prevent BOINC Manager from launching automatically when selected users log in.
</ul>

<h3>".tr(RLN_LINUX)."</h3>
".tr(RLN_LINUX_INSTALL_SEA_DESC)."
<p>"
.sprintf(tr(RLN_LINUX_DL_FILENAME),"boinc_5.2.13_i686-pc-linux-gnu.sh")." "
.sprintf(tr(RLN_LINUX_RUN_SEA),"sh boinc_5.2.13_i686-pc-linux-gnu.sh")
.tr(RLN_LINUX_RESULTOF_SEA)
."<dl>
<dt> boinc
<dd> ".tr(RLN_BOINC_CORE_CL)."
<dt> boincmgr
<dd> ".tr(RLN_BOINC_MANAGER)."
<dt>
run_client
<dd> ".tr(RLN_SCRIPT_RUN_CLIENT_DESC)."
<dt>
run_manager
<dd> ".tr(RLN_SCRIPT_RUN_MANAGER_DESC)."
</dl>

<p>
".sprintf(tr(RLN_LINUX_AUTOSTART),"<a href=auto_start.php>","</a>")."

<a name=uninstall></a>
<h2>".tr(RLN_UNINSTALLING)."</h2>
<h3>".tr(RLN_MSWIN)."</h3>
".tr(RLN_MSWIN_UNINSTALL_DESC)."
<h3>Mac OS X</h3>
To completely remove (<b>uninstall</b>) BOINC from your Macintosh:
    <ul>
    <li> Move the following files to the trash:
        <ul>
        <li> <b>BOINCManager.app</b> (from your <b>/Applications/</b> folder)
        <li> <b>BOINCSaver.saver</b> (from your <b>/Library/Screen Savers/</b> folder)  
        <li> the <b>BOINC Data/</b> folder (from your<b>/Library/Applications Support/</b> folder.)
        <li> <b>[username]/Library/Preferences/BOINC Manager Preferences</b>) 
        </ul>
    <li> Open the <b>Accounts</b> pane in the <b>System Preferences</b> (accessible from the Apple menu), and remove BOINCManager from your list of <b>Login Items</b> (or <b>Startup Items</b> under OS 10.3.)
    <li> Open <b>System Preferences</b> from the Apple menu. Select <b>Desktop & Screen Saver</b> and select a different screen saver.
    </ul>

<a name=issues></a>
<h2>".tr(RLN_KNOWN_ISSUES)."</h2>
<ul>
<li> ".tr(RLN_ISSUE_PROXY_NTLMAUTH)."
</ul>
<h3>".tr(RLN_MSWIN)."</h3>
<ul>
<li>
".sprintf(tr(RLN_ISSUE_MSWIN_LATEST_DIRECTX),"<a href=directx.php>","</a>")."
<li> 
".tr(RLN_ISSUE_MSWIN_SCREENSAVER_XP3D)."
<li> 
".tr(RLN_ISSUE_MSWIN_NO_SCREENSAVER)."
</ul>

<a name=troubleshoot></a>
<h2>".tr(RLN_TROUBLESHOOT)."</h2>
".tr(RLN_TROUBLESHOOT_INTRO)."
<ul>
<li> 
".tr(RLN_TROUBLESHOOT_PRJ_SPECIFIC)."
<li> 
".tr(RLN_TROUBLESHOOT_PROBLEM_PERSIST)."
<li> 
".sprintf(tr(RLN_TROUBLESHOOT_BOINC_ITSELF),"<a href=dev/>","</a>")."
</ul>

";
page_tail(true);
?>
