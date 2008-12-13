<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/cache.inc");


start_cache(3600);
page_head(tra("Download BOINC add-on software"));
echo "
    <p>" .
    tra("You can download applications in several categories.") ."
    <ul>
    <li>".
    tra("These applications are not endorsed by %1 and you use them at your own risk.", PROJECT) ."
    <li>" .
    tra("We do not provide instructions for installing these applications.
However, the author may have provided some help on installing or uninstalling the application. 
If this is not enough you should contact the author.").
    tra("Instructions for installing and running BOINC are %1here%2.", "<a href=http://boinc.berkeley.edu/participate.php>", "</a>")
    . "<li>" . 
    tra("This list is managed centrally at %1the BOINC website%2.", "<a href=\"http://boinc.berkeley.edu/addons.php\">", "</a>") ."
    </ul>
";

$httpFile = @fopen("http://boinc.berkeley.edu/addons.php?strip_header=true", "rb");
if (!$httpFile){
    echo "";
} else {
    fpassthru($httpFile);
    fclose($httpFile);
}

echo "
    <p><p>
";
page_tail();
end_cache(3600);
?>
