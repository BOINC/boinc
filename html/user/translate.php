<?
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
Obtain the project-specific translation files.
For SETI@home these are
<a href=http://setiathome2.ssl.berkeley.edu/cgi-bin/cvsweb.cgi/seti_boinc_html/project_specific_translations/>here</a>.
The PHP files for the web site are
<a href=http://setiathome2.ssl.berkeley.edu/cgi-bin/cvsweb.cgi/seti_boinc_html/>here</a>.
<li> Contact Jens Seidler (jens at planet-seidler.de),
tell him what language you want to do, and get instructions from him.
</ul>

<p>
You can also help
<a href=http://boinc.berkeley.edu/language.php>translate the BOINC client software</a>.
<p>

There is an email list
<a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_loc>boinc_loc at ssl.berkeley.edu</a> for people doing translations of the BOINC client software
and web interfaces.
";

page_tail();
?>
