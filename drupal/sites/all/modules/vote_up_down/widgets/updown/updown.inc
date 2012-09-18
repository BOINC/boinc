<?php

/**
 * @file
 * Up Down Widget.
 */

/**
 * Implementation of hook_vud_widgets(). (Specialized)
 */
function vud_updown_vud_widgets() {
  return array(
    'title' => t('Default'),
    'widget template' => 'widget',
  );
}
