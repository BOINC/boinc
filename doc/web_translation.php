<?php

require_once("docutil.php");

page_head("Web site translation");

echo "
BOINC has a mechanism for translating project web sites
into different languages.

<h3>How it works</h3>

<ul>
<li>
Pages to be translated must be PHP, and must include
<pre>
require_once(\"../inc/translation.inc\");
</pre>
<li>
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
<li>
The directory html/user/translations contains a number of
'translation files'.
These have names like 'da.po' (Danish) and 'en.po' (English).
Translation files are in PO format, which
is described
<a href=http://www.gnu.org/software/gettext/manual/html_node/gettext_9.html#SEC9>here</a>.
It's very simple: for example:
<table>
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

<li> When a person accesses the page, the browser passes a list of languages.
The BOINC PHP code finds the first of these languages for which
a translation is available, or English if none.
As the page is generated, each call to <code>tr()</code>
replaces the token with the corresponding translated text.

</ul>
<p>
In developing web pages, keep in mind that word
order differs between languages,
so you should avoid breaking a sentence up into
multiple translation units.
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

<h3>Project-specific translations</h3>
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

<h3>Doing a translation</h3>
<p>
If you are interested in doing translation for BOINC or
for a specific project:

<ul>
<li> Contact the translation manager.
For BOINC this is Robi Buecheler (rf.buecheler at switzerland.org).
Robi is also managing translations for SETI@home.
For other projects, contact the project.
<li>
Obtain (typically via CVS) the 'authoritative' translation file.
Typically this is en.po.
The translation files for BOINC can be found
<a href=http://boinc.berkeley.edu/cgi-bin/cvsweb.cgi/boinc/html/languages/translations/>here</a>.
<li>
Create a translation file for your language.
You can do this using a text editor or a specialized tool such as
<a href=http://www.poedit.org/>poedit</a>.
Send this to the translation manager,
who will then install it on the project's web site.
Check all relevant pages and fix as needed.
<li>
Because web sites are dynamic,
you will have to periodically update your translation.
You can do this efficiently by looking at the CVS diffs
of the authoritative translation file.
</ul>

";

page_tail();
?>
