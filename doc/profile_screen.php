<?php

require_once("docutil.php");

page_head("Profile screening and UOTD selection");

echo "
Users can create profiles containing text and/or pictures.
These profiles are displayed in various ways:
as a picture gallery, sorted by country, and sorted alphabetically.
The default project web site also shows a
<b>User of the Day</b> (UOTD) on its front page.
<p>
Depending on your project's requirements and resources,
there are two main approaches to screening profiles and selecting UOTD.
Both involve a 'profile screening page' used by project staff.
<ul>
<li> <b>Loose screening</b>:
    Only profiles with nonzero recent average are screened.
    The UOTD is selected randomly from among approved profiles.
    Profile pictures are shown independently of whether
    they have been screened.
    This is the default.
<li> <b>Tight screening</b>:
    All profiles are screened.
    A profile's picture is shown only if the project has approved it.
    The UOTD is selected randomly from among approved
    profiles that have nonzero recent average credit.
    To use tight screening, include the line
    ".html_text("
<profile_screening>1</profile_screening>")."
    in your <a href=configuration.php>config.xml file</a>.
</ul>

<p>
You can customize the database queries used to select
profiles for screening and for UOTD candidacy.
To do so, define either of the functions
<pre>
uotd_candidates_query()
profile_screen_query()
</pre>
in your html/project/project.inc file.
Each function must return a SQL query.
";

page_tail();
?>
