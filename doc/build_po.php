#!/usr/bin/php
<?php

// generate en.po
//
// NOTE: after running this, move LANG_NAME_NATIVE and LANG_NAME_INTERNATIONAL
// to the top, and set their strings to "English"
// (should automate this)

$path = ".";

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

HDR;

$out = fopen("BOINC-Web.pot", "w");
fwrite($out, $header);
$pipe = popen("xgettext --omit-header -o - --keyword=tra -L PHP --no-location $path/*.inc $path/*.php", "r");
stream_copy_to_stream($pipe, $out);
fclose($pipe);
fclose($out);

// msgen duplicates everything.
// Don't need this.  Just rename to en.po

//system("msgen -o $path/html/languages/translations/en.po $path/html/languages/translations/web.pot");

echo "Created BOINC-Web.pot.  Move it to ../locale/templates\n";
?>
