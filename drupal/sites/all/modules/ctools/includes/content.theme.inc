<?php
// $Id: content.theme.inc,v 1.3 2009/05/11 22:01:42 merlinofchaos Exp $

/**
 * @file
 * Contains theme registry and theme implementations for the content types.
 */

/**
 * Implementation of hook_theme to load all content plugins and pass thru if
 * necessary.
 */
function ctools_content_theme(&$theme) {
  ctools_include('content');

  $plugins = ctools_get_content_types();
  foreach ($plugins as $plugin) {
    if ($function = ctools_plugin_get_function($plugin, 'hook theme')) {
      $function($theme, $plugin);
    }
  }
}
