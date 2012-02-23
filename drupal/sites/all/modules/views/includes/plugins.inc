<?php
// $Id: plugins.inc,v 1.156.2.2 2009/11/30 23:55:33 merlinofchaos Exp $
/**
 * @file plugins.inc
 * Built in plugins for Views output handling.
 *
 */

/**
 * Implementation of hook_views_plugins
 */
function views_views_plugins() {
  $path = drupal_get_path('module', 'views') . '/js';

  $plugins = array(
    'module' => 'views', // This just tells our themes are elsewhere.
    'display' => array(
      'parent' => array(
        // this isn't really a display but is necessary so the file can
        // be included.
        'no ui' => TRUE,
        'handler' => 'views_plugin_display',
        'parent' => '',
      ),
      'default' => array(
        'title' => t('Defaults'),
        'help' => t('Default settings for this view.'),
        'handler' => 'views_plugin_display_default',
        'theme' => 'views_view',
        'no ui' => TRUE,
        'no remove' => TRUE,
        'js' => array('misc/collapse.js', 'misc/textarea.js', 'misc/tabledrag.js', 'misc/autocomplete.js', "$path/dependent.js"),
        'use ajax' => TRUE,
        'use pager' => TRUE,
        'use more' => TRUE,
        'accept attachments' => TRUE,
        'help topic' => 'display-default',
      ),
      'page' => array(
        'title' => t('Page'),
        'help' => t('Display the view as a page, with a URL and menu links.'),
        'handler' => 'views_plugin_display_page',
        'theme' => 'views_view',
        'uses hook menu' => TRUE,
        'use ajax' => TRUE,
        'use pager' => TRUE,
        'accept attachments' => TRUE,
        'admin' => t('Page'),
        'help topic' => 'display-page',
      ),
      'block' => array(
        'title' => t('Block'),
        'help' => t('Display the view as a block.'),
        'handler' => 'views_plugin_display_block',
        'theme' => 'views_view',
        'uses hook block' => TRUE,
        'use ajax' => TRUE,
        'use pager' => TRUE,
        'use more' => TRUE,
        'accept attachments' => TRUE,
        'admin' => t('Block'),
        'help topic' => 'display-block',
      ),
      'attachment' => array(
        'title' => t('Attachment'),
        'help' => t('Attachments added to other displays to achieve multiple views in the same view.'),
        'handler' => 'views_plugin_display_attachment',
        'theme' => 'views_view',
        'use ajax' => TRUE,
        'help topic' => 'display-attachment',
      ),
      'feed' => array(
        'title' => t('Feed'),
        'help' => t('Display the view as a feed, such as an RSS feed.'),
        'handler' => 'views_plugin_display_feed',
        'parent' => 'page', // so it knows to load the page plugin .inc file
        'uses hook menu' => TRUE,
        'use ajax' => FALSE,
        'use pager' => FALSE,
        'accept attachments' => FALSE,
        'admin' => t('Feed'),
        'help topic' => 'display-feed',
      ),
    ),
    'style' => array(
      'parent' => array(
        // this isn't really a display but is necessary so the file can
        // be included.
        'no ui' => TRUE,
        'handler' => 'views_plugin_style',
        'parent' => '',
      ),
      'default' => array(
        'title' => t('Unformatted'),
        'help' => t('Displays rows one after another.'),
        'handler' => 'views_plugin_style_default',
        'theme' => 'views_view_unformatted',
        'uses row plugin' => TRUE,
        'uses grouping' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
        'help topic' => 'style-unformatted',
      ),
      'list' => array(
        'title' => t('HTML List'),
        'help' => t('Displays rows as an HTML list.'),
        'handler' => 'views_plugin_style_list',
        'theme' => 'views_view_list',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
        'help topic' => 'style-list',
      ),
      'grid' => array(
        'title' => t('Grid'),
        'help' => t('Displays rows in a grid.'),
        'handler' => 'views_plugin_style_grid',
        'theme' => 'views_view_grid',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
        'help topic' => 'style-grid',
      ),
      'table' => array(
        'title' => t('Table'),
        'help' => t('Displays rows in a table.'),
        'handler' => 'views_plugin_style_table',
        'theme' => 'views_view_table',
        'uses row plugin' => FALSE,
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
        'help topic' => 'style-table',
      ),
      'default_summary' => array(
        'title' => t('List'),
        'help' => t('Displays the default summary as a list.'),
        'handler' => 'views_plugin_style_summary',
        'theme' => 'views_view_summary',
        'type' => 'summary', // only shows up as a summary style
        'uses options' => TRUE,
        'help topic' => 'style-summary',
      ),
      'unformatted_summary' => array(
        'title' => t('Unformatted'),
        'help' => t('Displays the summary unformatted, with option for one after another or inline.'),
        'handler' => 'views_plugin_style_summary_unformatted',
        'parent' => 'default_summary',
        'theme' => 'views_view_summary_unformatted',
        'type' => 'summary', // only shows up as a summary style
        'uses options' => TRUE,
        'help topic' => 'style-summary-unformatted',
      ),
      'rss' => array(
        'title' => t('RSS Feed'),
        'help' => t('Generates an RSS feed from a view.'),
        'handler' => 'views_plugin_style_rss',
        'theme' => 'views_view_rss',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'type' => 'feed',
        'help topic' => 'style-rss',
      ),
    ),
    'row' => array(
      'parent' => array(
        // this isn't really a display but is necessary so the file can
        // be included.
        'no ui' => TRUE,
        'handler' => 'views_plugin_row',
        'parent' => '',
      ),
      'fields' => array(
        'title' => t('Fields'),
        'help' => t('Displays the fields with an optional template.'),
        'handler' => 'views_plugin_row_fields',
        'theme' => 'views_view_fields',
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
        'help topic' => 'style-row-fields',
      ),
    ),
    'argument default' => array(
      // This type of plugin does not conform to the standard and
      // uses 'fixed' as the parent rather than having a separate parent.
      'fixed' => array(
        'title' => t('Fixed entry'),
        'handler' => 'views_plugin_argument_default',
      ),
      'php' => array(
        'title' => t('PHP Code'),
        'handler' => 'views_plugin_argument_default_php',
        'parent' => 'fixed',
      ),
    ),
    'argument validator' => array(
      'parent' => array(
        'no ui' => TRUE,
        'handler' => 'views_plugin_argument_validate',
        'parent' => '',
      ),
      'php' => array(
        'title' => t('PHP Code'),
        'handler' => 'views_plugin_argument_validate_php',
      ),
      'numeric' => array(
        'title' => t('Numeric'),
        'handler' => 'views_plugin_argument_validate_numeric',
      ),
    ),
    'access' => array(
      'parent' => array(
        'no ui' => TRUE,
        'handler' => 'views_plugin_access',
        'parent' => '',
      ),
      'none' => array(
        'title' => t('None'),
        'help' => t('Will be available to all users.'),
        'handler' => 'views_plugin_access_none',
        'help topic' => 'access-none',
      ),
      'role' => array(
        'title' => t('Role'),
        'help' => t('Access will be granted to users with any of the specified roles.'),
        'handler' => 'views_plugin_access_role',
        'uses options' => TRUE,
        'help topic' => 'access-role',
      ),
      'perm' => array(
        'title' => t('Permission'),
        'help' => t('Access will be granted to users with the specified permission string.'),
        'handler' => 'views_plugin_access_perm',
        'uses options' => TRUE,
        'help topic' => 'access-perm',
      ),
    ),
    'cache' => array(
      'parent' => array(
        'no ui' => TRUE,
        'handler' => 'views_plugin_cache',
        'parent' => '',
      ),
      'none' => array(
        'title' => t('None'),
        'help' => t('No caching of Views data.'),
        'handler' => 'views_plugin_cache_none',
        'help topic' => 'cache-none',
      ),
      'time' => array(
        'title' => t('Time-based'),
        'help' => t('Simple time-based caching of data.'),
        'handler' => 'views_plugin_cache_time',
        'uses options' => TRUE,
        'help topic' => 'cache-time',
      ),
    ),
  );

  if (module_invoke('ctools', 'api_version', '1.3')) {
    $plugins['style']['jump_menu_summary'] = array(
      'title' => t('Jump menu'),
      'help' => t('Puts all of the results into a select box and allows the user to go to a different page based upon the results.'),
      'handler' => 'views_plugin_style_summary_jump_menu',
      'theme' => 'views_view_summary_jump_menu',
      'type' => 'summary', // only shows up as a summary style
      'uses options' => TRUE,
      'help topic' => 'style-summary-jump-menu',
    );
    $plugins['style']['jump_menu'] = array(
      'title' => t('Jump menu'),
      'help' => t('Puts all of the results into a select box and allows the user to go to a different page based upon the results.'),
      'handler' => 'views_plugin_style_jump_menu',
      'theme' => 'views_view_jump_menu',
      'uses row plugin' => TRUE,
      'uses fields' => TRUE,
      'uses options' => TRUE,
      'type' => 'normal',
      'help topic' => 'style-jump-menu',
    );
  }

  return $plugins;
}

/**
 * Builds and return a list of all plugins available in the system.
 *
 * @return Nested array of plugins, grouped by type.
 */
function views_discover_plugins() {
  $cache = array('display' => array(), 'style' => array(), 'row' => array(), 'argument default' => array(), 'argument validator' => array(), 'access' => array(), 'cache' => array());
  // Get plugins from all mdoules.
  foreach (module_implements('views_plugins') as $module) {
    $function = $module . '_views_plugins';
    $result = $function();
    if (!is_array($result)) {
      continue;
    }

    $module_dir = isset($result['module']) ? $result['module'] : $module;
    // Setup automatic path/file finding for theme registration
    if ($module_dir == 'views') {
      $theme_path = drupal_get_path('module', $module_dir) . '/theme';
      $theme_file = 'theme.inc';
      $path = drupal_get_path('module', $module_dir) . '/plugins';
    }
    else {
      $theme_path = $path = drupal_get_path('module', $module_dir);
      $theme_file = "$module.views.inc";
    }

    foreach ($result as $type => $info) {
      if ($type == 'module') {
        continue;
      }
      foreach ($info as $plugin => $def) {
        $def['module'] = $module_dir;
        if (!isset($def['theme path'])) {
          $def['theme path'] = $theme_path;
        }
        if (!isset($def['theme file'])) {
          $def['theme file'] = $theme_file;
        }
        if (!isset($def['path'])) {
          $def['path'] = $path;
        }
        if (!isset($def['file'])) {
          $def['file'] = $def['handler'] . '.inc';
        }
        if (!isset($def['parent'])) {
          $def['parent'] = 'parent';
        }
        // merge the new data in
        $cache[$type][$plugin] = $def;
      }
    }
  }

  // Let other modules modify the plugins.
  drupal_alter('views_plugins', $cache);
  return $cache;
}

/**
 * Abstract base class to provide interface common to all plugins.
 */
class views_plugin extends views_object {
  /**
   * Init will be called after construct, when the plugin is attached to a
   * view and a display.
   */
  function init(&$view, &$display) {
    $this->view = &$view;
    $this->display = &$display;
  }

  /**
   * Provide a form to edit options for this plugin.
   */
  function options_form(&$form, &$form_state) { }

  /**
   * Validate the options form.
   */
  function options_validate(&$form, &$form_state) { }

  /**
   * Handle any special handling on the validate form.
   */
  function options_submit(&$form, &$form_state) { }

  /**
   * Add anything to the query that we might need to.
   */
  function query() { }

  /**
   * Provide a full list of possible theme templates used by this style.
   */
  function theme_functions() {
    return views_theme_functions($this->definition['theme'], $this->view, $this->display);
  }

  /**
   * Provide a list of additional theme functions for the theme information page
   */
  function additional_theme_functions() {
    $funcs = array();
    if (!empty($this->definition['additional themes'])) {
      foreach ($this->definition['additional themes'] as $theme => $type) {
        $funcs[] = views_theme_functions($theme, $this->view, $this->display);
      }
    }
    return $funcs;
  }

  /**
   * Validate that the plugin is correct and can be saved.
   *
   * @return
   *   An array of error strings to tell the user what is wrong with this
   *   plugin.
   */
  function validate() { return array(); }
}

