<?php
error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/translation.inc");
buildLanguages($lang_language_dir,$lang_translations_dir, $lang_compiled_dir);
buildLanguages($lang_language_dir,$lang_prj_translations_dir, $lang_compiled_dir, true);

echo "update_translations finished\n";
?>
