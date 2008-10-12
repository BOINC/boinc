<?php

require_once("../inc/translation.inc");
buildLanguages($lang_language_dir,$lang_translations_dir, $lang_compiled_dir);
buildLanguages($lang_language_dir,$lang_prj_translations_dir, $lang_compiled_dir, true);

?>