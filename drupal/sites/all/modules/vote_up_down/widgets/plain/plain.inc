<?php

/**
 * @file
 * Plain VUD Widget.
 */

/**
 * Implementation of hook_vud_widgets(). (Specialized)
 */
function vud_plain_vud_widgets() {
  return array(
    'title' => t('Plain'),
    'widget template' => 'widget',
    'votes template' => 'votes',
  );
}
