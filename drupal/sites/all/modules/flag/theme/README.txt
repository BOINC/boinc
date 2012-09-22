
Theming instructions
====================

You may want to visit the Theming Guide of this module, at:

  http://drupal.org/node/295346

Template file
-------------
In order to customize flag theming:

- Copy the 'flag.tpl.php' template file into your theme's folder.[1]

- Clear your theme registry.[2]

- Edit that copy to your liking.


Template variants[3]
-----------------
In addition, the theme layer will first look for the template
'flag--<FLAG_NAME>.tpl.php' before it turns to 'flag.tpl.php'. This too
you should place in your theme's folder.[2][1]


Footnotes
---------
[1] Or to a sub-folder in your theme's folder.

[2] Clearing the theme registry makes Drupal aware of your new template
file. This step is needed if you create or rename template files. This
step *isn't* needed if you merely modify the contents of a file. Instructions
on how to clear you theme registry are at http://drupal.org/node/173880#theme-registry

[3] For template variants to work correctly you must use Drupal 6.3 or above (or
apply the patch from http://drupal.org/node/241570).
