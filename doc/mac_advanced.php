<?php
require_once("docutil.php");
page_head("Standard Mac GUI installation");
echo "
<ul>
<li> If your browser has not already done so,
expand the zip archive by double-clicking on it in the Finder.
<li> Double-click on BOINC.pkg to run the installer, then follow the prompts.
<li> Close the installer when it is finished.  This will automatically launch the BOINC Manager.
<li> The installer does the following:
    <ul>
    <li> places BOINCManager.app in your <b>/Applications</b> folder, with the BOINC client embedded inside the BOINCManager application's bundle.
    <li> puts BOINCSaver.saver in your <b>/Library/Screen Savers</b> folder.  
    <li> sets BOINCManager as one of the items to automatically start whenever the user is logged in.  You can add or remove <b>Login Items</b> by using the <b>Accounts</b> pane in the <b>System Preferences</b> (accessible from the Apple menu).  
    <li> creates a <b>BOINC Data</b> folder inside your
<b>/Library/Applications Support</b> folder if one does not already exist.  If you have previously been running BOINC in a different folder, copy your data into this folder.  
    <li> puts BOINC's localization files in the <b>/Library/Applications Support/BOINC Data/locale</b> folder.  These files allow BOINC to support over two dozen languages. 
    </ul>
</ul>
";
page_tail();
?>
