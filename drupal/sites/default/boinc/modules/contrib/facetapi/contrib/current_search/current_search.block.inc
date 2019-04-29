<?php

/**
 * @file
 * Block hook implementations and block form alterations.
 */

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Adds the searcher visibility settings to the block form.
 */
function current_search_form_block_admin_configure_alter(&$form, &$form_state) {
  if ('current_search' == $form['module']['#value']) {

    $form['visibility']['current_search'] = array(
      '#type' => 'fieldset',
      '#title' => t('Search page'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#group' => 'visibility',
      '#weight' => -5,
      '#attached' => array(
        'js' => array(drupal_get_path('module', 'current_search') . '/current_search.js'),
      ),
    );

    // Gets the default value for this block.
    $searcher = db_query("SELECT searcher FROM {block_current_search} WHERE delta = :delta", array(
      ':delta' => $form['delta']['#value'],
    ))->fetchField();

    $form['visibility']['current_search']['searcher'] = array(
      '#type' => 'radios',
      '#title' => t('Search page'),
      '#options' => current_search_get_searcher_options(),
      '#description' => t('Select the search page this block is active on.'),
      '#default_value' => ($searcher) ? $searcher : current_search_get_default_searcher(),
    );

    // Adds submit handler to save the searcher data.
    $form['#submit'][] = 'current_search_form_block_admin_configure_submit';
  }
}

/**
 * Form submit handler for block configuration form.
 *
 * @see current_search_form_block_admin_configure_alter()
 */
function current_search_form_block_admin_configure_submit($form, &$form_state) {
  $values = $form_state['values'];
  current_search_set_block_searcher($values['delta'], $values['searcher']);
}

/**
 * Implements hook_block_info().
 */
function current_search_block_info() {
  $blocks = array();

  // Loads settings for enabled facets.
  ctools_include('export');
  foreach (ctools_export_crud_load_all('current_search') as $config) {
    if (empty($config->disabled)) {
      $blocks[$config->name] = array(
        'info' => 'Current search: ' . $config->label,
        //'cache' => DRUPAL_NO_CACHE,
      );
    }
  }

  // Returns available blocks.
  return $blocks;
}

/**
 * Implements hook_block_list_alter().
 *
 * Enforces visibility settings.
 */
function current_search_block_list_alter(&$blocks) {
  foreach ($blocks as $bid => $block) {
    if ('current_search' == $block->module) {
      if (!current_search_check_visibility($block->delta)) {
        unset($blocks[$bid]);
      }
    }
  }
}

/**
 * Returns the content for a facet based on the delta.
 */
function current_search_block_view($delta = '') {
  // Test block visibility if not already tested. This is necessary when using
  // modules such as Context that do not invoke hook_block_list_alter().
  $map = &ctools_static('current_search_delta_map');
  if (NULL === $map && !current_search_check_visibility($delta)) {
    return;
  }

  // Gets searcher from delta map.
  $searcher = $map[$delta];

  // Makes sure the adapter and configuration can be loaded.
  $adapter = facetapi_adapter_load($searcher);
  if ($adapter && ($config = ctools_export_crud_load('current_search', $delta))) {
    $build = array();

    // Iterates over configs and executes the plugins.
    foreach ($config->settings['items'] as $name => $settings) {
      if ($class = ctools_plugin_load_class('current_search', 'items', $settings['id'], 'handler')) {
        $plugin = new $class($name, $config);
        if ($return = $plugin->execute($adapter)) {
          $build[$name] = $return;
          $build[$name]['#theme_wrappers'][] = 'current_search_item_wrapper';
          $build[$name]['#current_search_id'] = $settings['id'];
        }
      }
    }

    // Returns the block content.
    if ($build) {
      $build['#contextual_links'] = array(
        'current_search' => array('admin/settings/current_search/list', array($delta)),
      );
      return array(
        'subject' => t('Current search'),
        'content' => $build,
      );
    }
  }
}

/**
 * Sets the block searcher for a configuration.
 *
 * @param $name
 *   A string containing the machine readable name of the configuration. The
 *   name also doubles as the block delta.
 * @param $searcher
 *   The machine readable name of the searcher.
 */
function current_search_set_block_searcher($name, $searcher) {
  // Deletes current search data.
  $query = "DELETE FROM {block_current_search} WHERE delta = '%s'";
  db_query($query, array($name));

  // Inserts new data into database.
  $query = "INSERT INTO {block_current_search} (delta, searcher) VALUES ('%s', '%s')";
  db_query($query, array($name, $searcher));
}

/**
 * Checks whether the block should be displayed.
 *
 * In cases where modules like Context are being used, hook_block_list_alter()
 * is not invoked and we get fatal errors. We have to test whether or not the
 * hook has been invoked and call this function manually otherwise.
 *
 * @param $delta
 *   The block delta.
 *
 * @return
 *   A boolean flagging whether to display this block or not.
 */
function current_search_check_visibility($delta) {

  // Caches the delta map, defaults to NULL so we can test whether this function
  // was called in hook_block_list_alter() or not.
  $map = &ctools_static('current_search_delta_map');
  if (NULL === $map) {
    $map = array();
    $result = db_query('SELECT delta, searcher FROM {block_current_search}');
    foreach ($result as $record) {
      $map[$record->delta] = $record->searcher;
    }
  }

  // Apply default if necessary.
  if (empty($map[$delta])) {
    $map[$delta] = current_search_get_default_searcher();
  }

  // Checks whether block should be displayed.
  if (!facetapi_is_active_searcher($map[$delta])) {
    return FALSE;
  }
  if (!$adapter = facetapi_adapter_load($map[$delta])) {
    return FALSE;
  }
  if (!$adapter->searchExecuted($map[$delta])) {
    return FALSE;
  }
  if (!$config = current_search_item_load($delta)) {
    return FALSE;
  }

  // Returns TRUE based on the empty_searches setting and the current search.
  switch ($config->settings['advanced']['empty_searches']) {
    case CURRENT_SEARCH_DISPLAY_KEYS:
      return ($adapter->getSearchKeys());
    case CURRENT_SEARCH_DISPLAY_FILTERS:
      return ($adapter->getAllActiveItems());
    case CURRENT_SEARCH_DISPLAY_KEYS_FILTERS:
      return ($adapter->getSearchKeys() || $adapter->getAllActiveItems());
    case CURRENT_SEARCH_DISPLAY_ALWAYS:
    default:
      return TRUE;
  }
}

/**
 * Gets the default searcher.
 *
 * @return
 *   The default searcher.
 *
 * @todo Figure out a beter default system.
 */
function current_search_get_default_searcher() {
  $options = current_search_get_searcher_options();
  return key($options);
}
