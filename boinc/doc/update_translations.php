#!/usr/bin/env php
<?php
error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../html/inc/translation.inc");
$lang_log_level = 0;
buildLanguages($lang_language_dir,$lang_translations_dir, $lang_compiled_dir);

echo "update_translations finished\n";
?>
