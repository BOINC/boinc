<?php
// $Id: utility.inc,v 1.1.2.2 2010/08/27 23:47:17 merlinofchaos Exp $

/**
 * @file
 * Contains general utility functions for CTools that do not need to be
 * in the module file.
 *
 * In particular, things that are only needed during hook_menu() and
 * hook_theme() are placed here.
 */

/**
 * Provide a hook passthrough to included files.
 *
 * To organize things neatly, each CTools tool gets its own toolname.$type.inc
 * file. If it exists, it's loaded and ctools_$tool_$type() is executed.
 * To save time we pass the $items array in so we don't need to do array
 * addition. It modifies the array by reference and doesn't need to return it.
 */
function _ctools_passthrough(&$items, $type = 'theme') {
  $files = drupal_system_listing('.' . $type . '.inc$', drupal_get_path('module', 'ctools') . '/includes', 'name', 0);
  foreach ($files as $file) {
    require_once './' . $file->filename;
    list($tool) = explode('.', $file->name, 2);

    $function = 'ctools_' . str_replace ('-', '_', $tool) . '_' . $type;
    if (function_exists($function)) {
      $function($items);
    }
  }
}

/**
 * Implementation of hook_theme_registry_alter()
 */
function ctools_theme_registry_alter(&$registry) {
  if ($registry['menu_local_tasks']['function'] == 'theme_menu_local_tasks') {
    $registry['menu_local_tasks'] = array(
      'function' => 'ctools_theme_menu_local_tasks',
      'path' => drupal_get_path('module', 'ctools') . '/includes',
      'file' => 'menu.inc',
    ) + $registry['menu_local_tasks'];
  }

  if (isset($registry['help']['function']) && $registry['help']['function'] == 'theme_help') {
    $registry['help'] = array(
      'function' => 'ctools_menu_help',
      'path' => drupal_get_path('module', 'ctools') . '/includes',
      'file' => 'menu.inc',
    ) + $registry['help'];
  }

  // Handle a special override for garland because it's cute and does its own
  // thing with tabs and we can't ask users to edit a core theme for us.
  if ($registry['menu_local_tasks']['function'] == 'phptemplate_menu_local_tasks' &&
      $registry['menu_local_tasks']['theme paths'][1] == 'themes/garland') {
    $registry['menu_local_tasks'] = array(
      'function' => 'ctools_garland_menu_local_tasks',
      'path' => drupal_get_path('module', 'ctools') . '/includes',
      'file' => 'menu.inc',
    ) + $registry['menu_local_tasks'];
  }

  if (isset($registry['page']['preprocess functions'][2]) &&
      $registry['page']['preprocess functions'][2] == 'phptemplate_preprocess_page' &&
      $registry['page']['theme paths'][1] == 'themes/garland') {
    $registry['page']['preprocess functions'][2] = 'ctools_garland_preprocess_page';
  }

  // Move this one last last last so it can catch changes made by modules and themes.
  $key = array_search('ctools_preprocess_page', $registry['page']['preprocess functions']);
  if ($key) {
    unset($registry['page']['preprocess functions'][$key]);
  }
  $registry['page']['preprocess functions'][] = 'ctools_preprocess_page';
}

