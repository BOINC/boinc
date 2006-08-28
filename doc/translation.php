<?php

require_once("docutil.php");

page_head("Web and GUI translations");

echo "
BOINC has a mechanism for non-English translations of
<ul>
<li> Parts of this site (the <a href=download.php>Download</a>
    and <a href=poll.php>BOINC user survey</a> pages,
    and related pages)
<li> The BOINC Manager
<li> Parts of the BOINC-supplied portion of project web sites
<li> The project-specific parts of project web sites
</ul>

<h2>Instructions for volunteer translators</h2>
<p>
Translations are done by volunteers.
If you're interested in helping:

<ul>
<li> Email the translation manager.
For BOINC this is <b>translate at boinc.berkeley.edu</b>.
Use this address also for SETI@home translations.
For other projects, contact the project.
<li>
Obtain (typically via CVS) the 'authoritative' translation file.
Usually this is en.po (English).
<li>
Create a translation file for your language.
You can do this using a text editor or a specialized tool such as
<a href=http://www.poedit.org/>poedit</a>.
<li>
Send this to the translation manager,
who will then install it on the project's web site.
Check all relevant pages and fix as needed.
<li>
Subscribe to the
<a href=http://ssl.berkeley.edu/mailman/listinfo/boinc_loc>boinc_loc at ssl.berkeley.edu</a>
email list, which is for translation-related discussion and announcements.

<li>
Because web sites are dynamic,
you will have to periodically update your translation.
You can do this efficiently by looking at the CVS diffs
of the authoritative translation file.
</ul>

<h2>Translation files</h2>

<p>
All translations are based on <b>translation files</b>.
Translation files are in PO format, which
is described
<a href=http://www.gnu.org/software/gettext/manual/html_node/gettext_9.html#SEC9>here</a>.
These have names like 'da.po' (Danish) and 'en.po' (English).
It's very simple.
For example:
<table width=100%>
<tr><th>en.po (English)</th></tr>
<tr><td>
<pre>
msgid \"APPS_VERSION\"
msgstr \"Current version\"

msgid \"APPS_DESCRIPTION\"
msgstr \"\$PROJECT currently has the following applications. \"
\"When you participate in \$PROJECT, work for one or more \"
\"of these applications will be assigned to your computer. \"
\"The current version of the application will be downloaded \"
\"to your computer. This happens automatically; you don't have to do anything. \"


</pre>
</td></tr>
<tr><th>da.po (Danish)</th></tr>
<tr><td>
<pre>
msgid \"APPS_VERSION\"
msgstr \"Nuv&aelig;rende version\"

msgid \"APPS_DESCRIPTION\"
msgstr \"\$PROJECT har i &oslash;jeblikket f&oslash;lgende applikationer. \"
\"N&aring;r du deltager i \$PROJECT vil arbejde fra en eller flere \"
\"af disse applikationer blive tildelt din computer. \"
\"Den nuv&aelig;rende version af applikationen vil blive downloadet \"
\"til din computer. Dette sker automatisk - du beh&oslash;ver ikke at g&oslash;re noget.\"

</pre>
</td></tr></table>
Each translation has a token CHARSET whose value should be
the character set (e.g. iso-8859-1) used in the translation.

<p>
Here are links to the translation files for
<ul>
<li> <a href=http://setiathome.berkeley.edu/cgi-bin/cvsweb.cgi/boinc/languages/translations/>this web site</a>
<li> <a href=http://setiathome.berkeley.edu/cgi-bin/cvsweb.cgi/boinc/html/languages/translations/>the BOINC-supplied part of project web sites.
<li> <a href=http://setiathome.berkeley.edu/cgi-bin/cvsweb.cgi/boinc/locale/client/>the BOINC Manager</a>.
</ul>


<h2>Translatable web pages</h2>
Translatable web pages must be PHP, and must include
<pre>
require_once(\"../inc/translation.inc\");
</pre>
Literal text is replaced with <code>tr(TOKEN)</code>,
where TOKEN is a short string representing the text.
For example,
<pre>
page_head(\"Current version\");
</pre>
is replaced with
<pre>
page_head(tr(APPS_VERSION));
</pre>
For BOINC projects,
the directory html/user/translations contains a number of
'translation files'.
When a person accesses the page, the browser passes a list of languages.
The BOINC PHP code finds the first of these languages for which
a translation is available, or English if none.
As the page is generated, each call to <code>tr()</code>
replaces the token with the corresponding translated text.

</ul>
<p>
In developing web pages, keep in mind that word order differs between languages,
so you should avoid breaking a sentence up into multiple translation units.
For example, use constructs like
<pre>
msgid \"ACTIVATE_OR_CREATE\"
msgstr \"Already have an original 'Classic' account as of May 14, 2004? \"
    \"&lt;br>We've transferred it, just %sactivate it%s. \"
    \"%sOtherwise %screate a new account%s.\"
</pre>
with the corresponding PHP:
<pre>
printf(tr(ACTIVATE_OR_CREATE),
    \"&lt;a href=sah_email_form.php>\", \"&lt;/a>\",
    \"&lt;p>&lt;img border=0 src=images/arrow_right.gif width=9 height=7>\",
    \"&lt;a href=create_account_form.php>\", \"&lt;/a>\"
);
</pre>

<h2>Project-specific translations</h2>
<p>
The web site of a BOINC-based project involves both
<ul>
<li> BOINC pages, such as the forms for creating accounts.
These are part of the BOINC source code distribution,
and are updated periodically by BOINC.
<li> Project-specific pages (and BOINC pages that
are modified by the project).
</ul>
<p>
To allow translations of both types of pages,
a project can haves its own
'project-specific translation files'.
These are stored in a directory html/user/project_specific_translations.
Project-specific translation files override BOINC translation files.

<h2>BOINC Manager translations</h2>
<p>
Menu names and other text in the BOINC manager are stored in
files in <b>boinc/locale/client/</b>.

";

page_tail();
?>
