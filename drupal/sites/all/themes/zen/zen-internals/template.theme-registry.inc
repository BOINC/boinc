<?php
/**
 * @file
 * Contains infrequently used theme registry build functions.
 */

/**
 * Implements HOOK_theme().
 *
 * We are simply using this hook as a convenient time to do some related work.
 */
function _zen_theme(&$existing, $type, $theme, $path) {
  // Compute the conditional stylesheets.
  if (!module_exists('conditional_styles')) {
    include_once './' . _zen_path() . '/zen-internals/template.conditional-styles.inc';
    // _conditional_styles_theme() only needs to be run once.
    if ($theme == 'zen') {
      _conditional_styles_theme($existing, $type, $theme, $path);
    }
  }

  // Since we are rebuilding the theme registry and the theme settings' default
  // values may have changed, make sure they are saved in the database properly.
  zen_theme_get_default_settings($theme);

  // If we are auto-rebuilding the theme registry, warn about the feature.
  // Always display the warning in the admin section, otherwise limit to three
  // warnings per hour.
  if (function_exists('user_access') && user_access('administer site configuration') && theme_get_setting('zen_rebuild_registry') && $theme == $GLOBALS['theme'] && (arg(0) == 'admin' || flood_is_allowed($GLOBALS['theme'] . '_rebuild_registry_warning', 3))) {
    flood_register_event($GLOBALS['theme'] . '_rebuild_registry_warning');
    drupal_set_message(t('For easier theme development, the theme registry is being rebuilt on every page request. It is <em>extremely</em> important to <a href="!link">turn off this feature</a> on production websites.', array('!link' => url('admin/build/themes/settings/' . $GLOBALS['theme']))), 'warning', FALSE);
  }

  // Keep track of all the base themes.
  static $base_themes = array();
  $base_themes[] = $theme;

  // Add a "process" phase to come after the "preprocess" functions.
  if ($type == 'theme') {
    foreach (array_keys($existing) as $hook) {
      // Normally, preprocess functions are added to the registry after
      // HOOK_theme() returns, but if we add them first, they won't be re-added.
      if (function_exists($theme . '_preprocess')) {
        $existing[$hook]['preprocess functions'][] = $theme . '_preprocess';
      }
      if (function_exists($theme . '_preprocess_' . $hook)) {
        $existing[$hook]['preprocess functions'][] = $theme . '_preprocess_' . $hook;
      }
      // Add base theme process functions.
      foreach ($base_themes as $base_theme) {
        if (function_exists($base_theme . '_process')) {
          $existing[$hook]['preprocess functions'][] = $base_theme . '_process';
        }
        if (function_exists($base_theme . '_process_' . $hook)) {
          $existing[$hook]['preprocess functions'][] = $base_theme . '_process_' . $hook;
        }
      }
      // Add the theme process functions.
      if (function_exists($theme . '_process')) {
        $existing[$hook]['preprocess functions'][] = $theme . '_process';
      }
      if (function_exists($theme . '_process_' . $hook)) {
        $existing[$hook]['preprocess functions'][] = $theme . '_process_' . $hook;
      }
    }
  }

  // Manipulations only to be done by the base theme.
  if ($theme == 'zen') {
    // Insert zen_show_blocks_discovery() before template_preprocess_page().
    $existing['page']['preprocess functions'] = array_merge(array('zen_show_blocks_discovery'), $existing['page']['preprocess functions']);

    // Allow theme hook suggestions on theme_links.
    $existing['links']['pattern'] = 'links__';

    // The base theme registers region.tpl.php.
    $templates = array(
      'region' => array(
        'arguments' => array('elements' => NULL),
        'pattern' => 'region_',
        'path' => drupal_get_path('theme', 'zen') . '/templates',
        'template' => 'region',
        // drupal_find_theme_templates() requires this, so manually add it.
        'include files' => array(),
      ),
    );
    // We have to manually find template suggestions since phptemplate_theme()
    // is in charge of that, but runs before zen_theme().
    $templates += drupal_find_theme_templates($templates, '.tpl.php', $path);
    // We manually register the preprocess and process functions, since Drupal
    // won't auto-register template_preprocess or process functions.
    foreach (array_keys($templates) as $key) {
      $templates[$key]['preprocess functions'] = array(
        'template_preprocess',
        'zen_preprocess',
        'zen_preprocess_region',
        'zen_process',
      );
    }
    return $templates;
  }

  // Else return nothing.
  return array();
}

/**
 * Return the theme settings' default values from the .info and save them into the database.
 *
 * @param $theme
 *   The name of theme.
 */
function zen_theme_get_default_settings($theme) {
  $themes = list_themes();

  // Get the default values from the .info file.
  $defaults = !empty($themes[$theme]->info['settings']) ? $themes[$theme]->info['settings'] : array();

  if (!module_exists('domain_theme') && !empty($defaults)) {
    // Merge the defaults with the theme settings saved in the database.
    $settings = array_merge($defaults, variable_get('theme_'. $theme .'_settings', array()));
    // Save the settings back to the database.
    if (db_is_active()) {
      variable_set('theme_'. $theme .'_settings', $settings);
    }
    else {
      $GLOBALS['conf']['theme_'. $theme .'_settings'] = $settings;
    }
    // If the active theme has been loaded, force refresh of Drupal internals.
    if (!empty($GLOBALS['theme_key'])) {
      theme_get_setting('', TRUE);
    }
  }

  // Return the default settings.
  return $defaults;
}
