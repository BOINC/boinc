<?php
// The code for conditional stylesheets started out as a patch for Zen. Now that
// it has been spun out to its own separate module, It would be nice to prevent
// code drift between the Zen implementation and the conditional_styles.module,
// so Zen now includes an exact copy of conditonal_style module's:
// conditional_styles.theme.inc,v 1.6 2009/08/24 08:57:32 johnalbin Exp

/**
 * @file
 * Allows themes to add conditional stylesheets.
 *
 * @see http://msdn.microsoft.com/en-us/library/ms537512.aspx
 */

/**
 * Return paths for the theme and its base themes.
 *
 * @param $theme
 *   The name of the theme.
 * @return
 *   An array of all the theme paths.
 */
function conditional_styles_paths_to_basetheme($theme) {
  static $theme_paths;
  if (empty($theme_paths[$theme])) {
    $theme_paths[$theme] = array();
    $themes = list_themes();
    // Grab the paths from the base theme.
    if (!empty($themes[$theme]->base_theme)) {
      $theme_paths[$theme] = conditional_styles_paths_to_basetheme($themes[$theme]->base_theme);
    }
    $theme_paths[$theme][$theme] = dirname($themes[$theme]->filename);
  }
  return $theme_paths[$theme];
}

/**
 * When the theme registry is rebuilt, we also build the conditional stylesheets.
 */
function _conditional_styles_theme($existing, $type, $theme, $path) {
  // @TODO: For PHP 4 compatibility we use foreach (array_keys($array) AS $key).
  // When PHP 5 becomes required (Drupal 7.x), use the following faster
  // implementation: foreach ($array AS $key => &$value) {}

  // Process the conditional stylesheets for every active theme.
  global $language;
  $themes = list_themes();
  foreach (array_keys($themes) AS $theme) {
    // We only need to process active themes.
    if ($themes[$theme]->status || $GLOBALS['theme'] == $theme) {
      $paths = conditional_styles_paths_to_basetheme($theme);

      // Grab all the conditional stylesheets.
      $stylesheets = array();
      // Start with the base theme and travel up the chain to the active theme.
      foreach ($paths AS $theme_name => $path) {
        // Look at the conditional-stylesheets defined in the theme's .info file.
        if (!empty($themes[$theme_name]->info['conditional-stylesheets'])) {
          foreach ($themes[$theme_name]->info['conditional-stylesheets'] AS $condition => $css) {
            // Allow the theme to override its base themes' styles.
            foreach ($css AS $media => $files) {
              foreach ($files AS $file) {
                $stylesheets[$condition][$media][$file] = $path;
              }
            }
          }
        }
      }
      // Render the stylesheets to link elements.
      $conditional_styles = '';
      if (!empty($stylesheets)) {
        $query_string = '?'. substr(variable_get('css_js_query_string', '0'), 0, 1);
        $base_path = base_path();
        foreach ($stylesheets AS $condition => $css) {
          // Each condition requires its own set of links.
          $output = '';
          foreach ($css AS $media => $files) {
            foreach ($files AS $file => $path) {
              // Don't allow non-existent stylesheets to clutter the logs with 404.
              if (file_exists("./$path/$file")) {
                $output .= "<link type=\"text/css\" rel=\"stylesheet\" media=\"$media\" href=\"$base_path$path/$file$query_string\" />\n";
                if ($language->direction == LANGUAGE_RTL){
                  $file_rtl = str_replace('.css', '-rtl.css', $file);
                  if (file_exists("./$path/$file_rtl")) {
                    $output .= "<link type=\"text/css\" rel=\"stylesheet\" media=\"$media\" href=\"$base_path$path/$file_rtl$query_string\" />\n";
                  }
                }
              }
            }
          }
          if ($output) {
            $conditional_styles .= "<!--[$condition]>\n$output<![endif]-->\n";
          }
        }
      }
      // Save the stylesheets for later retrieval.
      if ($conditional_styles) {
        variable_set('conditional_styles_' . $theme, $conditional_styles);
      }
      else {
        variable_del('conditional_styles_' . $theme);
      }
    }
  }

  // Return nothing.
  return array();
}
