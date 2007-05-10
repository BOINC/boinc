<?php
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
  <Content type=\"url\" href=\"".URL_BASE."user_profile/uotd_gadget.html\" /> 
</Module>
";

?>
