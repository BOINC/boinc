<?php
// $Id: page_mission.inc,v 1.1.2.2 2010/02/02 21:49:21 merlinofchaos Exp $

/**
 * @file
 * Plugin to handle the 'page_mission' content type which allows the
 * mission statement of the site to be embedded into a panel.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t('Mission'),
  'single' => TRUE,
  'icon' => 'icon_page.png',
  'description' => t('Add the site mission statement as content.'),
  'category' => t('Page elements'),
  'render last' => TRUE,
);

/**
 * Output function for the 'page_mission' content type.
 *
 * Outputs the mission statement for the site.
 */
function ctools_page_mission_content_type_render($subtype, $conf, $panel_args) {
  $block = new stdClass();
  $block->content = filter_xss_admin(theme_get_setting('mission'));

  return $block;
}
