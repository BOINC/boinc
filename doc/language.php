<?
require_once("docutil.php");
page_head("Language customization of the work manager");
echo "
    Menu names and other text in the work manager are stored in
    a file called <i>language.ini</i>.
    The release uses American English.
    Many other languages are available.
    The BOINC distribution includes all current language files.
    To use a particular language file,
    just rename it to 'language.ini'.

    <p>
    To submit a new language file,
    please email it to Rom Walton (rwalton at ssl.berkeley.edu).
";
page_tail();
?>
