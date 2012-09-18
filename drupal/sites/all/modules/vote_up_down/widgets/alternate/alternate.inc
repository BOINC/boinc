<?php

/**
 * @file
 * Alternate VUD Widget.
 */

/**
 * Implementation of hook_vud_widgets(). (Specialized)
 */
function vud_alternate_vud_widgets() {
  return array(
    'title' => t('Alternate'),
    'widget template' => 'widget',
  );
}
