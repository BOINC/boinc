<?php

/**
 * @file
 * Provides views integration.
 */

/**
 * Implementation of hook_views_plugins().
 */
function jump_views_plugins() {
  return array(
    'style' => array(
      'jump_views' => array(
        'title' => t('Jump menu'),
        'theme' => 'jump_views_plugin_style',
        'help' => t('Displays items as a select list jump menu.'),
        'handler' => 'jump_views_plugin_style',
        'uses row plugin' => TRUE,
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
      ),
    ),
  );
}
