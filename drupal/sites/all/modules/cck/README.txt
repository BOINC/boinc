// $Id: README.txt,v 1.12.2.4 2008/10/28 01:42:48 yched Exp $

Content Construction Kit
------------------------

NOTE: Install the advanced_help module (http://drupal.org/project/advanced_help)
to access more help (writing still in progress...)

To install, place the entire CCK folder into your modules directory.
Go to Administer -> Site building -> Modules and enable the Content module and one or
more field type modules:

- text.module
- number.module
- userreference.module
- nodereference.module

Now go to Administer -> Content management -> Content types. Create a new
content type and edit it to add some fields. Then test by creating
a new node of your new type using the Create content menu link.

The included optionswidget.module provides radio and check box selectors
for the various field types.

The included fieldgroup.module allows you to group fields together
in fieldsets to help organize them.

A comprehensive guide to using CCK is available as a CCK Handbook
at http://drupal.org/node/101723.

Known incompatibilitie
----------------------

The Devel Themer module that ships with Devel is known to mess with CCK admin pages.
As a general rule, Devel Themer should only be switched on intermittently when doing
theme work on a specific page, and switched off immediately after that, for it adds
massive processing overhead.

Maintainers
-----------
The Content Construction Kit was originally developped by:
John Van Dyk
Jonathan Chaffer

Current maintainers:
Karen Stevenson
Yves Chedemois

