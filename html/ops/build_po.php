#!/usr/bin/php
<?php

// generate translation template "en.po"
//
// NOTE: in its current form, this generates the translation file
// for all files in html/user and html/inc.
// We use this for the standard BOINC pages,
// but it's not what you want for your project-specific pages.
//
// To produce a translation file for your pages,
// edit the definition of FILE_LIST line so that it includes only your pages

//
// NOTE: after running this, move LANG_NAME_NATIVE and LANG_NAME_INTERNATIONAL
// to the top, and set their strings to "English"
// (should automate this)

if (!isset($argv[1])) {
    die('Usage: build_po.php [PROJECT_PATH]');
}
$path = $argv[1];

$FILE_LIST = "$path/html/inc/*.inc $path/html/user/*.php $path/html/project.sample/*.*";

$date = strftime('%Y-%m-%d %H:%M %Z');
$header = <<<HDR
# BOINC web translation
# Copyright (C) 2008 University of California
#
# This file is distributed under the same license as BOINC.
#
# FileID  : \$Id\$
#
msgid ""
msgstr ""
"Project-Id-Version: BOINC \$Id\$\\n"
"Report-Msgid-Bugs-To: BOINC translation team <boinc_loc@boinc.berkeley.edu>\\n"
"POT-Creation-Date: $date\\n"
"Last-Translator: Generated automatically from source files\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=utf-8\\n"
"Content-Transfer-Encoding: 8bit\\n"
"X-Poedit-SourceCharset: utf-8\\n"


HDR;

$out = fopen("en.pot", "w");

fwrite($out, $header);

$pipe = popen(
    "xgettext --omit-header -o - --keyword=tra -L PHP $FILE_LIST",
    "r"
);
stream_copy_to_stream($pipe, $out);

fclose($pipe);
fclose($out);

?>
