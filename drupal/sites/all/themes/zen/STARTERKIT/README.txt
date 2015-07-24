BUILD YOUR OWN SUB-THEME
------------------------

*** IMPORTANT ***

* In Drupal 6, the theme system caches template files and which theme functions
  should be called. What that means is if you add a new theme or preprocess
  function to your template.php file or add a new template (.tpl.php) file to
  your sub-theme, you will need to rebuild the "theme registry." See
  http://drupal.org/node/173880#theme-registry

* Drupal 6 also stores a cache of the data in .info files. If you modify any
  lines in your sub-theme's .info file, you MUST refresh Drupal 6's cache by
  simply visiting the admin/build/themes page.


The base Zen theme is designed to be easily extended by its sub-themes. You
shouldn't modify any of the CSS or PHP files in the zen/ folder; but instead you
should create a sub-theme of zen which is located in a folder outside of the
root zen/ folder. The examples below assume zen and your sub-theme will be
installed in sites/all/themes/, but any valid theme directory is acceptable
(read the sites/default/default.settings.php for more info.)

  Why? To learn why you shouldn't modify any of the files in the zen/ folder,
  see http://drupal.org/node/245802

 1. Copy the STARTERKIT folder out of the zen/ folder and rename it to be your
    new sub-theme. IMPORTANT: The name of your sub-theme must start with an
    alphabetic character and can only contain lowercase letters, numbers and
    underscores.

    For example, copy the sites/all/themes/zen/STARTERKIT folder and rename it
    as sites/all/themes/foo.

      Why? Each theme should reside in its own folder. To make it easier to
      upgrade Zen, sub-themes should reside in a folder separate from their base
      theme.

 2. In your new sub-theme folder, rename the STARTERKIT.info.txt file to include
    the name of your new sub-theme and remove the ".txt" extension. Then edit
    the .info file by editing the name and description field.

    For example, rename the foo/STARTERKIT.info.txt file to foo/foo.info. Edit
    the foo.info file and change "name = Zen Sub-theme Starter Kit" to
    "name = Foo" and "description = Read..." to "description = A Zen sub-theme".

      Why? The .info file describes the basic things about your theme: its
      name, description, features, template regions, CSS files, and JavaScript
      files. See the Drupal 6 Theme Guide for more info:
      http://drupal.org/node/171205

    Then, visit your site's admin/build/themes to refresh Drupal 6's cache of
    .info file data.

 3. By default your new sub-theme is using a fixed-width layout. If you want a
    liquid layout for your theme, delete the unneeded layout-fixed.css and
    layout-fixed-rtl.css files and edit your sub-theme's .info file and replace
    the reference to layout-fixed.css with layout-liquid.css.

    For example, edit foo/foo.info and change this line:
      stylesheets[all][]   = css/layout-fixed.css
    to:
      stylesheets[all][]   = css/layout-liquid.css

      Why? The "stylesheets" lines in your .info file describe the media type
      and path to the CSS file you want to include. The format for these lines
      is:  stylesheets[MEDIA][] = path/to/file.css

    Then, visit your site's admin/build/themes to refresh Drupal 6's cache of
    .info file data.

    Alternatively, if you are more familiar with a different CSS layout method,
    such as Blueprint or 960.gs, you can replace the "css/layout-fixed.css" line
    in your .info file with a line pointing at your choice of layout CSS file.

 4. Edit the template.php and theme-settings.php files in your sub-theme's
    folder; replace ALL occurrences of "STARTERKIT" with the name of your
    sub-theme.

    For example, edit foo/template.php and foo/theme-settings.php and replace
    every occurrence of "STARTERKIT" with "foo".

    It is recommended to use a text editing application with search and
    "replace all" functionality.

 5. Log in as an administrator on your Drupal site and go to Administer > Site
    building > Themes (admin/build/themes) and enable your new sub-theme.

 6. Internet explorer has a nasty bug that limits you to 31 stylsheets total. To
    get around this limitation during theme development, download, install and
    configure the "IE CSS Optimizer" module.

    http://drupal.org/project/ie_css_optimizer

    On a production server, you should enable full optimization of the "Optimize
    CSS files" option on the Admin Performance page at
    admin/settings/performance.


Optional:

 7. MODIFYING ZEN CORE TEMPLATE FILES:
    If you decide you want to modify any of the .tpl.php template files in the
    zen folder, copy them to your sub-theme's folder before making any changes.
    And then rebuild the theme registry.

    For example, copy zen/templates/page.tpl.php to foo/templates/page.tpl.php.

 8. THEMEING DRUPAL'S SEARCH FORM:
    Copy the search-theme-form.tpl.php template file from the modules/search/
    folder and place it in your sub-theme's folder. And then rebuild the theme
    registry.

    You can find a full list of Drupal templates that you can override in the
    templates/README.txt file or http://drupal.org/node/190815

      Why? In Drupal 6 theming, if you want to modify a template included by a
      module, you should copy the template file from the module's directory to
      your sub-theme's directory and then rebuild the theme registry. See the
      Drupal 6 Theme Guide for more info: http://drupal.org/node/173880

 9. FURTHER EXTENSIONS OF YOUR SUB-THEME:
    Discover further ways to extend your sub-theme by reading Zen's
    documentation online at:
      http://drupal.org/node/193318
    and Drupal 6's Theme Guide online at:
      http://drupal.org/theme-guide
