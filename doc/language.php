<?
require_once("docutil.php");
page_head("Language customization of the work manager");
echo "
    Menu names and other text in the work manager are stored in
    a file called <i>language.ini</i>.
    The release uses American English.
    Many other languages are available;
    a complete list is
    <a href=http://www.boinc.dk/index.php?page=download_languages>here</a>
    (thanks to Robi Buechler and other volunteers for this).

    <p>
    The BOINC distribution now includes all current language files.
    To use a particular language file,
    just rename it to 'language.ini'.
";
page_tail();
?>
