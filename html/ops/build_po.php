#!/usr/bin/php
<?php

if (!isset($argv[1])) {
    die('Usage: build_po.php [PROJECT_PATH]');
}
$path = $argv[1];

$date = strftime('%Y-%m-%d %H:%M %Z');
$header = <<<HDR
# BOINC web translation
# Copyright (C) 2008 University of California
#
# This file is distributed under the same license as BOINC.
#
# FileID  : \$Id\$
msgid ""
msgstr ""
"Project-Id-Version: BOINC \$Id\$\\n"
"Report-Msgid-Bugs-To: BOINC translation team <translate@boinc.berkeley.edu>\\n"
"POT-Creation-Date: $date\\n"
"Last-Translator: Generated automatically from source files\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=utf-8\\n"
"Content-Transfer-Encoding: 8bit\\n"
"X-Poedit-SourceCharset: utf-8\\n"

msgid "LANG_NAME_NATIVE"
msgstr "English"

msgid "LANG_NAME_INTERNATIONAL"
msgstr "English"


HDR;

$out = fopen("$path/html/languages/translations/web.pot", "w");
fwrite($out, $header);
$pipe = popen("xgettext --omit-header -o - --keyword=tra -L PHP --no-location $path/html/inc/*.inc $path/html/user/*.php $path/html/project.sample/*.*", "r");
stream_copy_to_stream($pipe, $out);
fclose($pipe);
fclose($out);

system("msgen -o $path/html/languages/translations/en.po $path/html/languages/translations/web.pot");
?>
