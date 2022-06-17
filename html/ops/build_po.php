#!/usr/bin/php
<?php

// generate translation template "en.po" for project-specific pages
//
// Run this in project_root/html/.
// edit the definition of FILE_LIST line so that it includes only your pages
// (not BOINC-supplied pages)

//$FILE_LIST = "user/index.php project/project.inc";

$cli_only = true;
require_once("../inc/util_ops.inc");

if (!isset($FILE_LIST)) {
    echo "You must edit build_po.php to specify your project's .php files\n";
    exit;
}

$date = date(DATE_RFC2822);
$header = <<<HDR
# PROJECT translation
# Copyright (C) PROJECT
#
# This file is distributed under the same license as BOINC.
#
msgid ""
msgstr ""
"Project-Id-Version: PROJECT"
"Report-Msgid-Bugs-To: BOINC translation team <boinc_loc@ssl.berkeley.edu>\\n"
"POT-Creation-Date: $date\\n"
"Last-Translator: Generated automatically from source files\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=utf-8\\n"
"Content-Transfer-Encoding: 8bit\\n"
"X-Poedit-SourceCharset: utf-8\\n"


HDR;

$out = fopen("en.po", "w");

fwrite($out, $header);

$pipe = popen(
    "xgettext --omit-header -o - --keyword=tra -L PHP $FILE_LIST",
    "r"
);
stream_copy_to_stream($pipe, $out);

fclose($pipe);
fclose($out);

?>
