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

check_get_args(array());

page_head(tra("Allowed HTML tags"));

echo tra("The following HTML tags are allowed in team descriptions:")."
<ul>
<li> &lt;b> or &lt;strong> (".tra("bold").")
<li> &lt;i> or &lt;em> (".tra("italics").")
<li> &lt;a> (".tra("hyperlink").")
<li> &lt;p> (".tra("paragraph").")
<li> &lt;br> (".tra("break").")
<li> &lt;pre> (".tra("preformatted").")
<li> &lt;img> (".tra("image; height cannot exceed 450 pixels. Please do not link to images without permission of the web site where the image is hosted.").")
</ul>
".tra("You can also use ampersand notation for special characters.");

page_tail();
?>
