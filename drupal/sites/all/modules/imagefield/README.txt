// $Id: README.txt,v 1.5 2009/03/16 23:34:17 quicksketch Exp $

ImageField provides an "Image" widget type to CCK. This module leverages the
functionality of FileField and behaves nearly identically. ImageField widgets
will give you a nice thumbnail preview of the image when uploaded, and provides
a few display options (formatters) within CCK to display the images when the
content is viewed.

ImageField was written by Darrel Opry (dopry).
Maintained by Nathan Haug (quicksketch) and Andrew Morton (drewish).

Dependencies
------------
 * FileField
 * Content

ImageField also provides additional features when used with the following:

 * Token (Generate dynamic paths when saving images.)
 * ImageCache (Create thumbnails of images on output.)

Install
-------

1) Copy the imagefield folder to the modules folder in your installation.

2) Enable the module using Administer -> Site building -> Modules
   (/admin/build/modules).

3) Create a new image field in through CCK's interface. Visit Administer ->
   Content management -> Content types (admin/content/types), then click
   Manage fields on the type you want to add an image upload field. Select
   "File" as the field type and "Image" as the widget type to create a new
   field.
