<?php

/**
 * @file
 * Plugin to handle the 'page' content type which allows the standard page
 * template variables to be embedded into a panel.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'single' => TRUE,
  'title' => t('Page footer message'),
  'icon' => 'icon_page.png',
  'description' => t('Add the page footer message as content.'),
  'category' => t('Page elements'),
  'render last' => TRUE,
);

/**
 * Output function for the 'page_footer_message' content type.
 *
 * Outputs the page footer message of the current page.
 */
function ctools_page_footer_message_content_type_render($subtype, $conf, $panel_args) {
  $block = new stdClass();
  $block->content = filter_xss_admin(variable_get('site_footer', FALSE));

  return $block;
}