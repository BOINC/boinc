// $Id: README.txt,v 1.2 2009/11/08 18:28:49 johnalbin Exp $

Your theme can add JavaScript files in two ways:

1. To add a JavaScript file to all pages on your website, edit your sub-theme's
   .info file and add a line like this one:

     scripts[] = js/my-jquery-script.js

2. To add a JavaScript file depending on a certain condition, you can add it
   using some PHP code in a preprocess function:

     drupal_add_js('js/my-jquery-script.js', 'theme');

   For the full documentation of drupal_add_js(), see:
     http://api.drupal.org/api/function/drupal_add_js
