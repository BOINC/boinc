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



require_once("../inc/util.inc");

page_head("Translations of the ".PROJECT." web site");

echo "
<p>
If you are fluent in English and another language,
you can help ".PROJECT." by translating parts of our web site
into your non-English language.
If you are interested in doing this:
<ul>
<li> Learn how BOINC's
<a href=http://boinc.berkeley.edu/translation.php>web site translation mechanism</a> works.
<li>
Contact project staff to obtain project-specific translation files,
and to get instructions for submitting translations into your language.
</ul>

<p>
You can also help
<a href=http://boinc.berkeley.edu/translation.php>translate the BOINC client software</a>.
<p>

There is an email list
<a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_loc>boinc_loc at ssl.berkeley.edu</a> for people doing translations of the BOINC client software
and web interfaces.
";

page_tail();
?>
