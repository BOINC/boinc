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

require_once("../inc/xml.inc");
require_once("../project/project.inc");

xml_header();

echo "
<Module>
  <ModulePrefs
     title=\"".PROJECT." User of the Day\"
     author=\"BOINC project\"
     author_email=\"".SYS_ADMIN_EMAIL."\"
     author_affiliation=\"".COPYRIGHT_HOLDER."\"
     author_location=\"Unknown\"
     description=\"Shows today's User of the Day for the BOINC-based distributed
                   computing project ".PROJECT."\"
     height=\"100\"
  />
  <Content type=\"url\" href=\"".secure_url_base()."user_profile/uotd_gadget.html\" />
</Module>
";

?>
