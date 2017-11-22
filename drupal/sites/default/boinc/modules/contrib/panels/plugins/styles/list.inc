<?php


/**
 * @file
 * Definition of the 'list' panel style.
 */

// Plugin definition
$plugin = array(
  'title' => t('List'),
  'description' => t('Presents the panes in the form of an HTML list.'),
  'render region' => 'panels_list_style_render_region',
  'settings form' => 'panels_list_style_settings_form',
);

/**
 * Render callback.
 *
 * @ingroup themeable
 */
function theme_panels_list_style_render_region($display, $region_id, $panes, $settings) {
  $items = array();

  foreach ($panes as $pane_id => $item) {
    if (isset($item)) {
      $items[] = $item;
    }
  }

  if (empty($settings['list_type'])) {
    $settings['list_type'] = 'ul';
  }

  return theme('item_list', $items, NULL, $settings['list_type']);
}

/**
 * Settings form callback.
 */
function panels_list_style_settings_form($style_settings) {
  $form['list_type'] = array(
    '#type' => 'select',
    '#title' => t('List type'),
    '#options' => array(
      'ul' => t('Unordered'),
      'ol' => t('Ordered'),
    ),
    '#default_value' => (isset($style_settings['list_type'])) ? $style_settings['list_type'] : 'ul',
  );

  return $form;
}

