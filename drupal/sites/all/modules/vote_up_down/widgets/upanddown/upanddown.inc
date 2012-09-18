<?php

/**
 * @file
 * Up Down Widget.
 */

/**
 * Implementation of hook_vud_widgets(). (Specialized)
 */
function vud_upanddown_vud_widgets() {
  return array(
    'title' => t('Up and down'),
    'widget template' => 'widget',
  );
}
