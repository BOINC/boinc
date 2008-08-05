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

page_head("Allowed HTML tags");

echo "
The following HTML tags are allowed in profiles, team descriptions,
signatures, etc.:
<ul>
<li> &lt;b> or &lt;strong> (bold)
<li> &lt;i> or &lt;em> (italics)
<li> &lt;a> (hyperlink)
<li> &lt;p> (paragraph)
<li> &lt;br> (break)
<li> &lt;pre> (preformatted)
<li> &lt;img> (image; height cannot exceed 450 pixels.
Please do not link to images without
permission of the web site where the image is hosted.)
</ul>
You can also use ampersand notation for special characters.
";

page_tail();
?>
