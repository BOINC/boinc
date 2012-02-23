Please read this file and also the INSTALL.txt.  
They contain answers to many common questions.
If you are developing for this module, the API.txt may be interesting.
If you are upgrading, check the CHANGELOG.txt for major changes.

**Description:
The Pathauto module provides support functions for other modules to 
automatically generate aliases based on appropriate criteria, with a 
central settings path for site administrators.

Implementations are provided for core content types: nodes, taxonomy 
terms, and users (including blogs and tracker pages).

Pathauto also provides a way to delete large numbers of aliases.  This feature 
is available at  Administer > Site building > URL aliases > Delete aliases

**Benefits:
Besides making the page address more reflective of its content than
"node/138", it's important to know that modern search engines give 
heavy weight to search terms which appear in a page's URL. By 
automatically using keywords based directly on the page content in the URL, 
relevant search engine hits for your page can be significantly
enhanced.

**Installation AND Upgrades:
See the INSTALL.txt file.

**Notices:
Pathauto just adds URL aliases to nodes, users, and taxonomy terms. Because 
it's an alias, the standard Drupal URL (for example node/123 or 
taxonomy/term/1) will still function as normal.  If you have external links 
to your site pointing to standard Drupal URLs, or hardcoded links in a module, 
template, node or menu which point to standard Drupal URLs it will bypass the 
alias set by Pathauto.

There are reasons you might not want two URLs for the same content on your 
site. If this applies to you, please note that you will need to update any 
hard coded links in your nodes or blocks. 

If you use the "system path" (i.e. node/10) for menu items and settings like
that, Drupal will replace it with the url_alias.

For external links, you might want to consider the Path Redirect or 
Global Redirect modules, which allow you to set forwarding either per item or 
across the site to your aliased URLs. 

URLs (not) Getting Replaced With Aliases:
Please bear in mind that only URLs passed through Drupal's l() or url()
functions will be replaced with their aliases during page output. If a module
or your template contains hardcoded links, such as 'href="node/$node->nid"'
those won't get replaced with their corresponding aliases. Use the
Drupal API instead:

* 'href="'. url("node/$node->nid") .'"' or
* l("Your link title", "node/$node->nid")

See http://api.drupal.org/api/HEAD/function/url and 
http://api.drupal.org/api/HEAD/function/l for more information.

** Disabling Pathauto for a specific node type (or taxonomy)
When the pattern for a node type is left blank, the default pattern will be 
used. But if the default pattern is also blank, Pathauto will be disabled 
for that node type.

** Bulk Updates Must be Run Multiple Times:
As of 5.x-2.x Pathauto now performs bulk updates in a manner which is more 
likely to succeed on large sites.  The drawback is that it needs to be run 
multiple times.  If you want to reduce the number of times that you need to 
run Pathauto you can increase the "Maximum number of objects to alias in a 
bulk update:" setting under General Settings.

**WYSIWYG Conflicts - FCKEditor, TinyMCE, etc.
If you use a WYSIWYG editor, please disable it for the Pathauto admin page.  
Failure to do so may cause errors about "preg_replace" problems due to the <p>
tag being added to the "strings to replace".  See http://drupal.org/node/175772

**Credits:
The original module combined the functionality of Mike Ryan's autopath with
Tommy Sundstrom's path_automatic.

Significant enhancements were contributed by jdmquin @ www.bcdems.net.

Matt England added the tracker support.

Other suggestions and patches contributed by the Drupal community.

Current maintainers: 
  Greg Knaddison - http://growingventuresolutions.com
  Mike Ryan - http://mikeryan.name
  Frederik 'Freso' S. Olesen - http://freso.dk

**Changes:
See the CHANGELOG.txt file.

$Id: README.txt,v 1.16 2008/06/21 18:20:38 greggles Exp $
