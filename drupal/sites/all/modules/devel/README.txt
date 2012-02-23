README.txt
==========

A module containing helper functions for Drupal developers and
inquisitive admins. This module can print a log of
all database queries for each page request at the bottom of each page. The
summary includes how many times each query was executed on a page, and how long each query
 took.
 
 It also
 - a block for running custom PHP on a page
 - a block for quickly accessing devel pages
 - a block for masquerading as other users (useful for testing)
 - reports memory usage at bottom of page
 - more
 
 This module is safe to use on a production site. Just be sure to only grant
 'access development information' permission to developers.

Also a dpr() function is provided, which pretty prints arrays and strings. Useful during
development. Many other nice functions like dpm(), dvm().

AJAX developers in particular ought to install FirePHP Core from http://www.firephp.org/ and put it in the devel directory. Your path to fb.php should looks like devel/FirePHPCore/lib/FirePHPCore/fb.php. You can use svn checkout http://firephp.googlecode.com/svn/trunk/trunk/Libraries/FirePHPCore. Then you can log php variables to the firebug console. Is quite useful. 

Included in this package is also: 
- devel_themer.module which outputs deep information related to all theme calls on a page.
- devel_node_access module which prints out the node_access records for a given node. Also offers hook_node_access_explain for all node access modules to implement. Handy.
- devel_generate.module which bulk creates nodes, users, comment, terms for development

Macro module has moved to http://drupal.org/project/macro.

COMPATIBILITY NOTES
==================
- Modules that use AHAH may have incompatibility with the query log and other footer info. Consider setting $GLOBALS['devel_shutdown'] = FALSE in order to avoid issues.
-  Modules that use AJAX should idenify their response as Content-type: text/javascript. The easiest way to do that is run your reply through drupal_json().


AUTHOR/MAINTAINER
======================
-moshe weitzman
weitzman at tejasa DOT com
