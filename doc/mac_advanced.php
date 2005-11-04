<?php
require_once("docutil.php");
page_head("Standard Mac GUI installation");
echo "
<ul>
<li> If your browser has not already done so,
expand the zip archive by double-clicking on it in the Finder.
<li> Double-click on BOINC.pkg to run the installer.
<li> The installer places two items onto your hard drive: BOINCManager.app in your /Applications folder and BOINCSaver.saver in your /Library/Screen Savers folder.  
<li> It also sets BOINCManager as one of the items to automatically start whenever the user is logged in.  You can add or remove Startup Items by using the Accounts Pane in the System Preferences (accessible from the Apple menu).  
<li> The installer creates a \"BOINC Data\" folder inside your
\"/Library/Applications Support\" folder if one does not already exist.  If you have previously been running BOINC in a different folder, copy your data into this folder.  
<li> The BOINC application bundle includes the BOINC client embedded inside it.
<li> Close the installer when it is finished.  This will automatically launch the BOINC Manager.
</ul>
";
page_tail();
?>
