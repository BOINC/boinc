<?php

/**
 * @file
 * Definition of the 'block' panel style.
 */

// Plugin definition
$plugin = array(
  'title' => t('System block'),
  'description' => t('Display the pane as a system block; this is more restrictive than the default.'),
  'render pane' => 'panels_block_style_render_pane',
  'weight' => -10,
);

/**
 * Render callback.
 *
 * @ingroup themeable
 */
function theme_panels_block_style_render_pane($content, $pane, $display) {
  if (empty($content->content)) {
    return;
  }

  if (!empty($content->title)) {
    $content->subject = $content->title;
  }

  $content->region = $pane->panel;
  if (!isset($content->module)) {
    $content->module = $content->type;
  }
  if (!isset($content->delta)) {
    $content->delta = $content->subtype;
  }

  // If using per pane classes, $block->css_class will need to be added in your
  // preprocess or template, along with any other Panels specific field you
  // might want to utilize.
  return theme('block', $content);
}

