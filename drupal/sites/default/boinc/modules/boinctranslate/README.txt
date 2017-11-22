INTRODUCTION
-------------

The BOINC translate module contains the code to process BOINC project-specific content through Tranifex, a 3-rdparty translation service.

BOINC Translate Filter
----------------------

Included in this module is a input filter: boinctranslate_filter,
which helps in splitting the content (nodes) into chunks.

When a node is created or edited, the content is split into chunks
using a delimiter: #SPLIT_TOKEN#. Each chunk is hashed and save in the
drupal database- tables: locales_source and locales_target. The
location field in the table holds the context of the chunk:
node:(nid):body::(hash).

When the node is viewed, the boinctranslate_filter input filter is
called. It also splits the text into chunks. The delimiters are
removed from the output, but retained in the database content. Then
the chunks are hashed and compared to those found in the database. If
the hashes match, then the translated content is provided.

The workflow for an editor/admin is to add the token to the text at
semi-regular intervals. The choice is entirely up to the editor. S/he
can add or remove them as the text grows or shrinks. If no delimiter
is found, the entire content of the node body will be hashed as a
single block.

Translators should see the node body split into separate chunks. If
only one chunk changes, then only that one translation needs to be
updated.

When creating or editting nodes, there is an option: "Export for
translation". Only nodes which have this "Export for translation"
checked will be translated. This option is selected by default for new
nodes. This option is only available for the 'Page' content-type. It
can be added to other content types in Admin > Content Managnement >
Content Types > (Choose type) > Manage Fields > Add existing field:
select field_boinctranslate_transifex. Tip: Place this existing field
in a New group for better organization.

Make sure you hide this field by going to Admin > Content Managnement
> Content Types > (Choose type) > Display fields and hiding this field
in all instances.

Copy of the help page for this input filter:

Use this filter to split the body of a node into chunks to make
translations easier. Add the token '#SPLIT_TOKEN#', without the quotes
in the editor when creating or editing a Web page. The filter will
then split the text chunks using these tokens as delimiters. Each
chunk is translated separately. Example: if a long page as five
paragraphs, and each is a chunk, and paragraphs 2 and 3 are edited,
chunks 1, 4, and 5 will not require re-translation.

The editor/admin may add as many of these tokens as you wish. You may
add and remove them at your discretion. It is permitted for a page as
no tokens, as the entire content will be a single chunk- such as for a
shorter page.


