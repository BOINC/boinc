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
    <li> places BOINCManager.app in your /Applications folder, with the BOINC client embedded inside the BOINCManager application's bundle.
    <li> puts BOINCSaver.saver in your /Library/Screen Savers folder.  
    <li> sets BOINCManager as one of the items to automatically start whenever the user is logged in.  You can add or remove Startup Items by using the Accounts Pane in the System Preferences (accessible from the Apple menu).  
    <li> creates a \"BOINC Data\" folder inside your
\"/Library/Applications Support\" folder if one does not already exist.  If you have previously been running BOINC in a different folder, copy your data into this folder.  
    <li> puts BOINC's localization files in the \"BOINC Data/locale\" folder.  These files allow BOINC to support over two dozen languages. 
    </ul>
</ul>
";
page_tail();
?>
