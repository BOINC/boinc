// $Id: README.txt,v 1.6 2009/03/27 03:56:37 quicksketch Exp $

FileField provides an "File" field type to CCK. It provides many advantages over
the Drupal core "Upload" module including:

 * Per-field upload control (file extensions, file size).
 * Per-node upload size limits.
 * Multiple fields per content type.
 * Customizable paths for saving uploads (plus token support for dynamic paths).
 * Icons for uploaded file types.

FileField was written by Darrel Opry (dopry).
Maintained by Nathan Haug (quicksketch) and Andrew Morton (drewish).

Dependencies
------------
 * Content

FileField also provides additional features when used with the following:

 * ImageField (See an image preview during editing.)
 * Token (Generate dynamic paths when saving images.)
 * ImageCache (Create thumbnails of images on output.)

If your site is any larger than a personal blog, you should definitely install
the following modules to increase the security and stability of your uploads.

 * Transliteration (Convert unsafe characters to file system safe names.)
 * MimeDetect (Check the content of files to ensure they match the extension.)

Install
-------

1) Copy the filefield folder to the modules folder in your installation.

2) Enable the module using Administer -> Site building -> Modules
   (/admin/build/modules).

3) Create a new file field in through CCK's interface. Visit Administer ->
   Content management -> Content types (admin/content/types), then click
   Manage fields on the type you want to add an file upload field. Select
   "File" as the field type and "File" as the widget type to create a new
   field.

4) Upload files on the node form for the type that you set up.
