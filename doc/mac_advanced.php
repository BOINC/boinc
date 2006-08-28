<?php
require_once("docutil.php");
page_head("Standard Mac GUI installation");
echo "
<ul>
<li> If your browser has not already done so,
expand the zip archive by double-clicking on it in the Finder.
<li> Double-click on <b>BOINC.pkg</b> to run the installer, then follow the prompts.
<li> Close the installer when it is finished.  This will automatically launch the BOINC Manager.
<li> If you want BOINC to be your screen saver, open <b>System Preferences</b> from the Apple menu. Select <b>Desktop & Screen Saver</b> and select <b>BOINCSaver</b>.
<p>
<li> The installer does the following:
    <ul>
    <li> places BOINCManager.app in your <b>/Applications/</b> folder, with the BOINC client embedded inside the BOINCManager application's bundle.
    <li> puts BOINCSaver.saver in your <b>/Library/Screen Savers/</b> folder.  
    <li> creates a <b>BOINC Data/</b> folder inside your
<b>/Library/Applications Support/</b> folder if one does not already exist.  If you have previously been running BOINC in a different folder, copy your data into this folder.  
    <li> puts BOINC's localization files in the <b>/Library/Applications Support/BOINC Data/locale/</b> folder.  These files allow BOINC to support over two dozen languages. 
    <li> sets BOINCManager as one of the items to automatically start whenever the user is logged in.  You can add or remove <b>Login Items</b> by using the <b>Accounts</b> pane in the <b>System Preferences</b> (accessible from the Apple menu). (These are called <b>Startup Items</b> if you are running OS 10.3.)  
    </ul>
    <p>
<li> To completely remove (<b>uninstall</b>) BOINC from your Macintosh:
    <ul>
    <li> Move the following files to the trash:
        <ul>
        <li> BOINCManager.app (from your <b>/Applications/</b> folder)
        <li> BOINCSaver.saver (from your <b>/Library/Screen Savers/</b> folder)  
        <li> the <b>BOINC Data/</b> folder (from your<b>/Library/Applications Support/</b> folder.)
        <li> BOINC.pkg (from your <b>/Library/Receipts/</b> folder) 
        </ul>
    <li> Open the <b>Accounts</b> pane in the <b>System Preferences</b> (accessible from the Apple menu), and remove BOINCManager from your list of <b>Login Items</b> (or <b>Startup Items</b> under OS 10.3.)
    <li> Open <b>System Preferences</b> from the Apple menu. Select <b>Desktop & Screen Saver</b> and select a different screen saver.
    </ul>
</ul>
<p>
Several <a href=mac_admin_tools.php>tools</a> for Macintosh system administrators are available to:
<ul>
<li> automatically run BOINC as a daemon or system service at boot time
<li> implement improved security for stand-alone clients 
<li> prevent BOINC Manager from launching automatically when selected users log in.
</ul>
";
page_tail();
?>
