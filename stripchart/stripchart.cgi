#! /usr/bin/env perl

# The contents of this file are subject to the Mozilla Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is the Berkeley Open Infrastructure for Network Computing.
#
# The Initial Developer of the Original Code is the SETI@home project.
# Portions created by the SETI@home project are Copyright (C) 2002
# University of California at Berkeley. All Rights Reserved.
#
# Contributor(s):
#

# Stripchart.cgi - Version 2.1 by Matt Lebofsky  ( started: November 11, 2002 )
#
# Requires: stripchart
#           stripchart.cnf (make sure you set the path the .cnf file below)
#           apache or other CGI-enabled web server
#
# Stripchart.cgi is a web-based GUI interface for stripchart, allowing
# users to display multiple plots from various data sources.
#
# You should only need to edit the variables in the section
# "GLOBAL/DEFAULT VARS" below
#
# Type "stripchart -h" for usage
#
# See doc/stripchart.txt for details

use CGI;
$query = CGI::new();
use File::Basename;

################
# READ IN .cnf
################

# Where is the .cnf file?
$cnfpath = "./stripchart.cnf";

# Read it in:
open (CNFFILE,$cnfpath) or die "cannot open configuration file: $cnfpath\nmake sure this variable is set properly";
while (<CNFFILE>) { eval }
close (CNFFILE);

###############
# PREP ARRAYS
###############

# Read in list of datafiles:
@datafiles = `$grepexe -v '^#' $datafilelist`;

# Read in list of queries:
@querylist = `$grepexe -v '^#' $queryfilelist`;

# Make file option list based on @datafiles
$optionlist = "";
foreach $element (@datafiles) {
  chomp $element;
  $thisop = (split /:/, $element)[1];
  $optionlist .= "<option>$thisop</option>\n";
  }

# Make query option list based on @querylist
$queryoptionlist = "<option value=\"stripchart.cgi\">------------------------------</option>";
foreach $element (@querylist) {
  chomp $element;
  ($thisname,$thisquery) = (split /:/, $element)[0,1];
  $queryoptionlist .= "<option value=\"stripchart.cgi?$thisquery\">$thisname</option>\n";
  }

# Make year list based on time right now, and other lists
$yearlist = "";
for ($i = $year; $i> 1999; $i--) { $yearlist .= "<option>$i</option>\n" }
$monthlist = sprintf("<option>%02d</option>\n",$month);
foreach $i (01 .. 12) { $monthlist .= sprintf("<option>%02d</option>\n",$i) }
$daylist = sprintf("<option>%02d</option>\n",$day);
foreach $i (01 .. 31) { $daylist .= sprintf("<option>%02d</option>\n",$i) }
$hourlist = sprintf("<option>%02d</option>\n",$hour);
foreach $i (00 .. 23) { $hourlist .= sprintf("<option>%02d</option>\n",$i) }
$minlist = sprintf("<option>%02d</option>\n",$min);
for ($i=0; $i<59; $i+=5) { $minlist .= sprintf("<option>%02d</option>\n",$i) }

#############
# SUBS
#############

sub to_unix_time {
  # same routine as in stripchart

  # no colons and no decimal point? must already be in unix time
  if ($_[0] !~ /:/ && $_[0] !~ /\./ ) { return $_[0] }

  # colons = civil time
  if ($_[0] =~ /:/) {
    (my $year, my $month, my $day, my $hour, my $minute) = split /:/, $_[0];
    $month--;
    return timelocal(0,$minute,$hour,$day,$month,$year)
    }

  # must be julian date
  return (($_[0] - 2440587.5) * $daysecs);

  }

#############
# MAIN
#############

# ARE WE JUST PLOTTING A SINGLE GRAPH?
#
# stripchart.cgi calls itself via an "<IMG SRC=" tag, so we need to
# act accordingly - if all we have to do is call stripchart to plot
# a graph based on the user-selected flags, then do so and exit:

if ($query->param("flags") ne "") {
  $flags = $query->param("flags");
  $outfile = "/tmp/tempout$$" . "." . rand(100000);
  print "Pragma: nocache\nCache-Control: no-cache\nContent-type: image/gif\n\n";
  `$stripchartexe $flags > $outfile`;
  open (OUTFILE,"$outfile");
  while ($dummy=read(OUTFILE,$buffer,1024)) { print $buffer }
  close (OUTFILE);
  unlink ($outfile);
  exit 0
  }

# ARE WE SAVING/DELETING A QUERY?
#
# If the user chose to save or delete a query, act on that and
# then continue on with the standard plots:

if ($query->param("sqname") ne "") {
  $sqname = $query->param("sqname");
  # are we deleting it?
  if ($query->param("delq") eq "y") {
    @querylist = `$grepexe -v '^$sqname:' $queryfilelist`;
    open (QUERYLIST,">$queryfilelist");
    flock (QUERYLIST,2);
    foreach $queryline (@querylist) {
      print QUERYLIST $queryline
      }
    }
  # must be saving it
  else {
    # first check to see if query already in the list
    $found = 0;
    foreach $checkline (@querylist) {
      ($key,$value) = split /:/, $checkline;
      if ($key eq $sqname) { $found = 1 }
      }
    # not found - add it to the end
    if (!$found) {
      open (QUERYLIST,">>$queryfilelist");
      flock (QUERYLIST,2);
      $fullquery = $ENV{'QUERY_STRING'};
      $fullquery =~ s/sqname=$sqname//;
      print QUERYLIST "$sqname:$fullquery\n";
      close (QUERYLIST);
      }
    }
  }

# PARSE INCOMING
$datafile1 = $query->param("df1");

$numcharts = $query->param("numcharts");
if ($numcharts == 0) { $numcharts = $defaultnumcharts }

print $query->header;
print << "EOF";
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<title>Stripchart</title>
</head>
<body>
EOF

if ($datafile1 ne "") { # stripcharts were selected, so plot them

foreach $which (1 .. $numcharts) {
  $thisfile = $query->param("df$which");
  if ($thisfile eq "No plot" || $thisfile =~ /^-+$/ || $thisfile eq "") { next }
  if ($thisfile eq "Same as datafile 1") { $thisfile = $query->param("df1") }
  $rangetype = $query->param("dr$which");
  $trustnum = $which;
  $from = "-f ";
  $to = "";
  if ($rangetype eq "sa1") { $rangetype = $query->param("dr1"); $trustnum = 1 }
  if ($rangetype eq "lh") { $from .= "-" .($query->param("lhv$trustnum") / 24) }
  elsif ($rangetype eq "ld") { $from .= "-" . ($query->param("ldv$trustnum")) }
  else {
    $fromstring = $query->param("dfy$trustnum") . ":" .
                  $query->param("dfm$trustnum") . ":" .
                  $query->param("dfd$trustnum") . ":" .
                  $query->param("dfh$trustnum") . ":" .
                  $query->param("dfn$trustnum");
    $tostring = $query->param("dty$trustnum") . ":" .
                $query->param("dtm$trustnum") . ":" .
                $query->param("dtd$trustnum") . ":" .
                $query->param("dth$trustnum") . ":" .
                $query->param("dtn$trustnum");
    $from .= to_unix_time($fromstring);
    $to = "-t " . to_unix_time($tostring)
    }

  foreach $element (@datafiles) {
    ($maybepath,$maybefile,$maybedatacol,$maybeflags) = split /:/, $element;
    if ($maybefile eq $thisfile) {
      $fullpath = $maybepath;
      $datacol = $maybedatacol;
      $flags = $maybeflags;
      }
    }

  $flags .= " $from $to";

  if ($query->param("log$which")) { $flags .= " -L" }
  if ($query->param("bavg$which")) { $flags .= " -B " }
  if ($query->param("base$which")) { $flags .= " -b " . $query->param("basev$which") }
  if ($query->param("ymin$which")) { $flags .= " -d " . $query->param("yminv$which") }
  if ($query->param("ymax$which")) { $flags .= " -D " . $query->param("ymaxv$which") }
  if ($query->param("savem")) { $flags .= " -O /tmp/stripchart_plot_$which.gif" }


  $flags .= " -i $fullpath -y $datacol -T \"$thisfile RANGE\"  $defaultflags";

  $fixflags = $flags;
  $fixflags =~ s/ /+/g;
  $fixflags =~ s/"/%22/g;

  if ($cgiplotwidth == 0) {
    print "<img src=\"stripchart.cgi?flags=$fixflags\"><br>\n"
    }
  else {
    print "<img src=\"stripchart.cgi?flags=$fixflags\" width=$cgiplotwidth height=$cgiplotheight><br>\n"
    }

  }
print "<hr>";

  }

# Now display the user entry form:

print << "EOF";
<form name="Saved queries">
<font size=2>
Select a saved query if you like:
</font>
<font size=1>
<select onChange="location = this.options[selectedIndex].value">
$queryoptionlist
</select>
</font>
</form>
<font size=2>

Number of stripcharts:
EOF
for ($i=1;$i<21;$i++) {
  $fullquery = $ENV{'QUERY_STRING'};
  $fullquery =~ s/numcharts=\d+//;
  $fullquery =~ s/&+/&/g;
  $fullquery =~ s/^&//;
  print "<a href=\"stripchart.cgi?$fullquery&numcharts=$i\">$i</a>\n"
  }

print << "EOF";
<p>
<form action=stripchart.cgi>
<input type=submit value="click here" name=submit> to plot stripcharts -
enter name:
<input type=text name=sqname size=20>
to save query (check here:
<input type=checkbox name=delq value=y>
to delete)
<input type=hidden value=$numcharts name=numcharts>
<p>
EOF

foreach $which (1 .. $numcharts) {
  $thisoptionlist = $optionlist;
  if ($which > 1) {
    $thisoptionlist = "<option>No plot</option>\n" .
                      "<option>Same as datafile 1</option>\n" .
                      "<option>----------------------------</option>\n" . $optionlist
    }
  if ($dummy = $query->param("df$which")) {
    $dummy = "<option>$dummy</option>";
    $newoptionlist = "$dummy\n";
    foreach $thisline (split /\n/, $thisoptionlist) {
      if ($dummy ne $thisline) { $newoptionlist .= "$thisline\n" }
      }
    $thisoptionlist = $newoptionlist;
    }
  $lhc = "";
  $ldc = "";
  $ownc = "";
  $sa1c = "";
  if ($dummy = $query->param("dr$which")) {
    if ($dummy eq "lh") { $lhc = "checked" }
    elsif ($dummy eq "ld") { $ldc = "checked" }
    elsif ($dummy eq "own") { $ownc = "checked" }
    elsif ($dummy eq "sa1") { $sa1c = "checked" }
    }
  else {
    if ($which > 1) { $sa1c = "checked" }
    else { $lhc = "checked" }
    }
  $samerange = "";
  if ($which > 1) {
    $samerange = "<input type=radio name=dr$which value=sa1 $sa1c> Same range as datafile 1"
    }

  $logcheck = "";
  if ($query->param("log$which") eq "y") { $logcheck = "checked" }

  $bavgcheck = "";
  if ($query->param("bavg$which") eq "y") { $bavgcheck = "checked" }
  $basecheck = "";
  if ($query->param("base$which") eq "y") { $basecheck = "checked"; $baseval = $query->param("basev$which") }


  $ymincheck = "";
  $ymaxcheck = "";
  if ($query->param("ymin$which") eq "y") { $ymincheck = "checked"; $yminval = $query->param("yminv$which") }
  if ($query->param("ymax$which") eq "y") { $ymaxcheck = "checked"; $ymaxval = $query->param("ymaxv$which") }

  if ($dummy = $query->param("dfy$which")) {
    $dfytop = "<option>$dummy</option>\n<option>---</option>\n" }
  else { $dfytop = "" }
  if ($dummy = $query->param("dfm$which")) {
    $dfmtop = "<option>$dummy</option>\n<option>---</option>\n" }
  else { $dfmtop = "" }
  if ($dummy = $query->param("dfd$which")) {
    $dfdtop = "<option>$dummy</option>\n<option>---</option>\n" }
  else { $dfdtop = "" }
  if ($dummy = $query->param("dfh$which")) {
    $dfhtop = "<option>$dummy</option>\n<option>---</option>\n" }
  else { $dfhtop = "" }
  if ($dummy = $query->param("dfn$which")) {
    $dfntop = "<option>$dummy</option>\n<option>---</option>\n" }
  else { $dfntop = "" }

  if ($dummy = $query->param("dty$which")) {
    $dtytop = "<option>$dummy</option>\n<option>---</option>\n" }
  else { $dtytop = "" }
  if ($dummy = $query->param("dtm$which")) {
    $dtmtop = "<option>$dummy</option>\n<option>---</option>\n" }
  else { $dtmtop = "" }
  if ($dummy = $query->param("dtd$which")) {
    $dtdtop = "<option>$dummy</option>\n<option>---</option>\n" }
  else { $dtdtop = "" }
  if ($dummy = $query->param("dth$which")) {
    $dthtop = "<option>$dummy</option>\n<option>---</option>\n" }
  else { $dthtop = "" }
  if ($dummy = $query->param("dtn$which")) {
    $dtntop = "<option>$dummy</option>\n<option>---</option>\n" }
  else { $dtntop = "" }

  $lhourlist = "";
  if ($query->param("lhv$which")) {
    $lhourlist = "<option>" . $query->param("lhv$which") . "</option>";
    }
  foreach $hourval (24,36,48,60,72) {
    if ($hourval != $query->param("lhv$which")) {
      $lhourlist .= "<option>$hourval</option>";
      }
    }

  $ldaylist = "";
  if ($query->param("ldv$which")) {
    $ldaylist = "<option>" . $query->param("ldv$which") . "</option>";
    }
  foreach $dayval (2,3,4,5,6,7,10,14,21,28,30,60,90,120,240,360,720) {
    if ($dayval != $query->param("ldv$which")) {
      $ldaylist .= "<option>$dayval</option>";
      }
    }

print << "EOF";
please select datafile $which:
<select name=df$which size=1>
$thisoptionlist
</select>
<br>
<input type=radio name=dr$which value=lh $lhc> Last
<font size=1>
<select name=lhv$which size=1>
$lhourlist
</select>
</font>
<font size=2>
hours&nbsp;&nbsp;&nbsp;
<input type=radio name=dr$which value=ld $ldc> Last
</font>
<font size=1>
<select name=ldv$which size=1>
$ldaylist
</select>
</font>
<font size=2>
days&nbsp;&nbsp;&nbsp;
$samerange
<br>
<input type=radio name=dr$which value=own $ownc> Enter range:
</font>
<font size=1>
<select name=dfy$which size=1>$dfytop$yearlist
</select>/<select name=dfm$which size=1>$dfmtop$monthlist
</select>/<select name=dfd$which size=1>$dfdtop$daylist
</select> <select name=dfh$which size=1>$dfhtop$hourlist
</select>:<select name=dfn$which size=1>$dfntop$minlist
</select> ->
<select name=dty$which size=1>$dtytop$yearlist
</select>/<select name=dtm$which size=1>$dtmtop$monthlist
</select>/<select name=dtd$which size=1>$dtdtop$daylist
</select> <select name=dth$which size=1>$dthtop$hourlist
</select>:<select name=dtn$which size=1>$dtntop$minlist
</select>
</font>
<font size=2>
<br>
<input type=checkbox name=log$which value=y $logcheck> Log y axis?
<input type=checkbox name=bavg$which value=y $bavgcheck> Baseline average, or
<input type=checkbox name=base$which value=y $basecheck> Baseline at:
<input type=text name=basev$which value="$baseval" size=8>
<input type=checkbox name=ymin$which value=y $ymincheck> Y min:
<input type=text name=yminv$which value="$yminval" size=8>
<input type=checkbox name=ymax$which value=y $ymaxcheck> Y max:
<input type=text name=ymaxv$which value="$ymaxval" size=8>
</font>
<hr>
EOF

  } # end foreach $which

print << "EOF";
<font size=2>
<input type=checkbox name=savem value=y> Save images in /tmp?
<br>
<input type=submit value="click here" name=submit> to plot stripcharts
</td></tr></table>
</form><p>
<a href=stripchart.cgi>Reset Form</a><p>
</font>
<font size=1>
Stripchart version $majorversion.$minorversion by
<a href="mailto:mattl\@ssl.berkeley.edu">Matt Lebofsky</a>.
</body></html>
EOF
