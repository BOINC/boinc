<?php
// $Id: updown.inc,v 1.1.2.2 2010/05/01 08:18:23 marvil07 Exp $

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
