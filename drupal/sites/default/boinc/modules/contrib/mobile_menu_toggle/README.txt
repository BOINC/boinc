INTRODUCTION
------------

Mobile Menu Toggle

This module creates a block with a "Menu" link within it. When clicked
that link initiates a jQuery slideToggle effect to show or hide a user
selected menu.

Created By:
 * Kevin Basarab (@kBasarab)
 * Kendall Totten (@starryeyez024)

Modified for BOINC-Drupal by:
 * Shawn Kwang 

INSTALLATION
------------

This module should be included in the BOINC-Drupal project, and should
be automatically downloaded along with the other BOINC-Drupal modules.

Enable this module using the Administration pages: Administer > Site
Building > Modules.

Or use drush
 * 'drush en mobile_menu_toggle'

REQUIREMENTS
------------

This module requires the following module:

* DHTML Menu (https://www.drupal.org/project/dhtml_menu), version 6.x-4.x

CONFIGURATION
-------------

Configure module at: /admin/config/user-interface/mobile-menu-toggle

Insert "Mobile Menu Toggle" block in site where "menu" link should
appear. Use a CSS selector to choose which element will be toggled.
