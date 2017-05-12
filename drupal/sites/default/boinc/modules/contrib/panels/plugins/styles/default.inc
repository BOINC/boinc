<?php

/**
 * @file
 * Definition of the 'default' panel style.
 */

// Plugin definition
$plugin = array(
  'title' => t('No style'),
  'description' => t('The default panel rendering style; displays each pane with a separator.'),
  'render region' => 'panels_default_style_render_region',
  'weight' => -15,
);

/**
 * Render callback.
 *
 * @ingroup themeable
 */
function theme_panels_default_style_render_region($display, $region_id, $panes, $settings) {
  $output = '';

  $print_separator = FALSE;
  foreach ($panes as $pane_id => $pane_output) {
    // Add the separator if we've already displayed a pane.
    if ($print_separator) {
      $output .= '<div class="panel-region-separator"></div>';
    }

    $output .= $pane_output;
    // If we displayed a pane, this will become true; if not, it will become
    // false.
    $print_separator = (bool) $pane_output;
  }

  return $output;
}
