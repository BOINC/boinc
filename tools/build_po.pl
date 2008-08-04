#!/usr/bin/perl

use strict;
use warnings;

my $path = $ARGV[0];
if ($path eq "") {
    die "Usage: build_po.pl [PROJECT_PATH]";
}
system("xgettext --omit-header --directory=".$path."/html/inc -o en.po --keyword=tra -L PHP --no-location ".$path."/html/inc/*.inc");
system("xgettext --omit-header --directory=".$path."/html/user -j -o en.po --keyword=tra -L PHP --no-location ".$path."/html/user/*.php");
system("xgettext --omit-header --directory=".$path."/html/project -j -o en.po --keyword=tra -L PHP --no-location ".$path."/html/project/*.*");

my @timedata = localtime(time);
my $header = "";
$header .= "# Language: English (International)\n";
$header .= "# FileID  : \$Id\$\n";
$header .= "msgid \"\"\n";
$header .= "msgstr \"\"\n";
$header .= "PO-Revision-Date: ".($timedata[5]+1900)."-".($timedata[4]+1)."-".$timedata[3]." ".$timedata[2].":".$timedata[1]."\n";
$header .= "Last-Translator: Generated automatically from source files\n";
$header .= "MIME-Version: 1.0\n";
$header .= "Content-Type: text/plain; charset=utf-8\n";
$header .= "Content-Transfer-Encoding: 8bit\n";
$header .= "X-Poedit-SourceCharset: utf-8\n";
$header .= "\n\n";

my $content = "";
open (IN, "en.po");
while (my $line = <IN>) {
    $content .= $line;
}
close(IN);
open (OUT, ">en.po");
print OUT $header;
print OUT $content;
close(OUT);
