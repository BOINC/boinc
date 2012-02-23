/* $Id: README.TXT,v 1.1.4.3 2009/07/24 15:32:54 sun Exp $ */

-- SUMMARY --

Menu Trails implements primary/secondary links which keep the current menu trail
"active" or highlighted.  The module provides a means of broadly categorizing
nodes (by type or taxonomy) as falling "under" a known menu item.  These nodes
are not added to the menu tree (keeping the menu admin system sane) but they
will trigger the functionality above -- preserving navigation state for the user
-- when being viewed.

For a full description of the module, visit the project page:
  http://drupal.org/project/menutrails

To submit bug reports and feature suggestions, or to track changes:
  http://drupal.org/project/issues/menutrails


-- REQUIREMENTS --

None.


-- INSTALLATION --

* Install as usual, see http://drupal.org/node/70151 for further information.


-- CONFIGURATION --

* Classify nodes in Administer >> Site building >> Menus >> Trails.


-- TROUBLESHOOTING --

Menu Trails contains a built-in override for theme_links(), which is necessary
to add the "active" class to the containing <li> for each menu item.  If your
theme already includes an override, this may cause a PHP error, and you will
need to work out the differences between the two theme overrides yourself.

The 5.x version of the module required you to manually override theme_links() in
your theme's template.php file.

Note that if your theme already overrides the theme_links() function in
template.php file, then you need to manually reconcile the differences between
your THEMENAME_links() and Menu Trails' phptemplate_links() function.  The final
result should live in THEMENAME_links() in your template.php file.

Feel free to suggest better alternatives or other ways to tackle the issue if
you have them.


-- CONTACT --

Current maintainers:
* Daniel F. Kudwien (sun) - http://drupal.org/user/54136
* Stefan M. Kudwien (smk-ka) - http://drupal.org/user/48898


